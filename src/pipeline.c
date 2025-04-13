#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

static VkShaderModule vertexShaderModule;
static VkShaderModule fragmentShaderModule;

static VkDynamicState dynamicStates[] = {
	VK_DYNAMIC_STATE_VIEWPORT, 
	VK_DYNAMIC_STATE_SCISSOR, 
};

VkViewport viewport = { };
VkRect2D scissor = { };

VkPipelineLayout pipelineLayout;

int 
read_shader(const char* path, uint8_t** buffer, size_t* pBfSize) 
{
	int ret = EXIT_FAILURE;

	FILE* file = fopen(path, "r");
	if (!file) { return ret; }

	fseek(file, 0L, SEEK_END);
	long fSize = ftell(file);
	if (fSize == -1L) { goto out; }

	*buffer = (uint8_t*) malloc(fSize * sizeof(uint8_t));
	if (!*buffer) { goto out; }

	rewind(file);
	*pBfSize = fread(*buffer, sizeof(uint8_t), fSize, file);
	if (*pBfSize != fSize) { 
		*pBfSize = 0;
		free(*buffer);
		goto out; 
	}

	ret = EXIT_SUCCESS;
out:
	fclose(file);
	return ret;
}

VkResult 
create_shader_module(VkDevice device, 
		     VkShaderModule* shaderModule, 
		     const uint8_t* buffer, 
		     const size_t bfSize) 
{
	VkShaderModuleCreateInfo createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = bfSize;
	createInfo.pCode = (const uint32_t*)(buffer);

	if (vkCreateShaderModule(device, &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	return VK_SUCCESS;
}

VkResult 
create_graphics_pipeline(VkDevice device, 
			 VkExtent2D extent, 
			 VkRenderPass renderPass, 
			 VkPipeline* pGraphicsPipeline) 
{
	VkResult ret;

	uint8_t* vertexShader = nullptr;
	size_t vxShSize = 0;
	char* path = "shaders/vert.spv"; 
	ret = read_shader(path, &vertexShader, &vxShSize);
	if (ret != VK_SUCCESS) {
		fprintf(stderr, "Pipeline: failed to retrieve %s code.\n", path);
		goto out;
	}

	ret = create_shader_module(device, &vertexShaderModule, vertexShader, vxShSize);
	if (ret != VK_SUCCESS) {
		fputs("Pipeline: failed to create shader module.\n", stderr);
		goto out;
	}

	uint8_t* fragmentShader = nullptr;
	size_t fgShSize = 0;
	path = "shaders/frag.spv";
	ret = read_shader(path, &fragmentShader, &fgShSize);
	if (ret != EXIT_SUCCESS) {
		fprintf(stderr, "Pipeline: failed to retrieve %s code.\n", path);
		goto out;
	}

	ret = create_shader_module(device, &fragmentShaderModule, fragmentShader, fgShSize);
	if (ret != VK_SUCCESS) {
		fputs("Pipeline: failed to create shader module.\n", stderr);
		goto out;
	}

	VkPipelineShaderStageCreateInfo shaderStages[2]; 
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].flags = 0;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertexShaderModule;
	shaderStages[0].pName = "main";

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].flags = 0;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = fragmentShaderModule;
	shaderStages[1].pName = "main";

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { };
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineDynamicStateCreateInfo dynamicState = { };
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = (uint32_t) sizeof(dynamicStates) / sizeof(VkDynamicState);
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { };
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) extent.width;
	viewport.height = (float) extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewportState = { };
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = { };
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = { };
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = { };
	colorBlendAttachment.colorWriteMask = 
		VK_COLOR_COMPONENT_R_BIT | 
		VK_COLOR_COMPONENT_G_BIT | 
		VK_COLOR_COMPONENT_B_BIT | 
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = { };
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { };
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	ret = vkCreatePipelineLayout(device, 
				      &pipelineLayoutInfo, 
				      nullptr, 
				      &pipelineLayout);
	if (ret != VK_SUCCESS) {
		fputs("Pipeline: failed to create pipeline layout.\n", stderr);
		goto out;
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = { };
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	ret = vkCreateGraphicsPipelines(device, 
					VK_NULL_HANDLE, 
					1, 
					&pipelineInfo, 
				 	nullptr, 
					pGraphicsPipeline);
	if (ret != VK_SUCCESS) {
		fputs("Pipeline: failed to create graphics pipeline.\n", stderr);
		goto out;
	}

	ret = VK_SUCCESS;
out:
	if (vertexShader) { free(vertexShader); } 
	if (fragmentShader) { free(fragmentShader); }
	return ret;
}

void 
close_graphics_pipeline(VkDevice device, VkPipeline graphicsPipeline) 
{
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyShaderModule(device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
}
