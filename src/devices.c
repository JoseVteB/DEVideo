#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include "devices.h"
#include "pipeline.h"

static VkPhysicalDevice physicalDevice;
static VkPhysicalDeviceFeatures deviceFeatures;

typedef struct QueueFamilyIndices {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
	VkBool32 isComplete;
} QueueFamilyIndices;
static QueueFamilyIndices indices;

static VkDevice logicalDevice;
static VkQueue graphicsQueue;
static VkQueue presentQueue;

const char* const deviceExtensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
};

static VkSurfaceCapabilitiesKHR capabilities;

typedef struct Formats {
	uint32_t count;
	VkSurfaceFormatKHR* data;
} Formats;
static Formats formats;

typedef struct PresentModes {
	uint32_t count;
	VkPresentModeKHR* data;
} PresentModes;
static PresentModes presentModes;

static VkSwapchainKHR swapChain;

typedef struct SwapChainImages {
	uint32_t count;
	VkImage* data;
} SwapChainImages;
static SwapChainImages images;
static VkFormat swapChainFormat;
static VkExtent2D extent;

typedef struct SwapChainImgViews {
	uint32_t count;
	VkImageView* data;
} SwapChainImgViews;
static SwapChainImgViews views;

static VkRenderPass renderPass;
static VkPipeline graphicsPipeline;

typedef struct SwapChainFramebuffers {
	uint32_t count;
	VkFramebuffer* data;
} SwapChainFramebuffers;
static SwapChainFramebuffers swapChainFramebuffers;

static VkCommandPool commandPool;
static VkCommandBuffer commandBuffer;

VkSemaphore imageAvailableSph;
VkSemaphore renderFinishedSph;
VkFence inFlightFence;

void 
find_queue_families(VkPhysicalDevice device, 
		    VkSurfaceKHR surface,
		    QueueFamilyIndices* pIndices) 
{
	pIndices->isComplete = false;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	VkQueueFamilyProperties queueFamilies[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	VkBool32 hasGraphicsFamily = false;
	VkBool32 supportsPresent = false;
	for (uint32_t i = 0; i < queueFamilyCount; ++i) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			pIndices->graphicsFamily = i;
			hasGraphicsFamily = true;
		}

		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportsPresent);

		if ((pIndices->isComplete = hasGraphicsFamily && supportsPresent)) { break; }
		hasGraphicsFamily = false;
		supportsPresent = false;
	}
}

bool 
check_device_extension_support(VkPhysicalDevice device) 
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	VkExtensionProperties availableExtensions[extensionCount];
	vkEnumerateDeviceExtensionProperties(device, 
					      nullptr, 
					      &extensionCount, 
					      availableExtensions);

	size_t requiredCount = sizeof(deviceExtensions) / sizeof(const char*);
	for (size_t i = 0; i < requiredCount; ++i) {
		bool match = false;
		for (size_t j = 0; j < extensionCount; ++j) {
			if (strcmp(deviceExtensions[i], 
	      				availableExtensions[j].extensionName) == 0) {
				match = true;
				break;
			}
		}
		if (!match) { return false; }
	}

	return true;
}

void 
query_swapChain_support(VkPhysicalDevice device, VkSurfaceKHR surface) 
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

	if (formats.count) {
		free(formats.data);
		formats.data = nullptr;
		formats.count = 0;
	}
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formats.count, nullptr);
	if (formats.count != 0) {
		formats.data = (VkSurfaceFormatKHR*) 
				malloc(formats.count * sizeof(VkSurfaceFormatKHR));
		formats.count = formats.count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, 
						       surface, 
						       &formats.count, 
						       formats.data);
	}

	if (presentModes.count) {
		free(presentModes.data);
		formats.data = nullptr;
		presentModes.count = 0;
	}
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, 
						   surface, 
						   &presentModes.count, 
						   nullptr);
	if (presentModes.count != 0) {
		presentModes.data = (VkPresentModeKHR*) 
			malloc(presentModes.count * sizeof(VkPresentModeKHR));
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, 
							    surface, 
							    &presentModes.count, 
							    presentModes.data);
	}
}

bool 
is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface) 
{
	find_queue_families(device, surface, &indices);

	bool extensionSupport = check_device_extension_support(device);

	query_swapChain_support(device, surface);
	bool swapChainSupport = (formats.count > 0) && (presentModes.count > 0);

	return indices.isComplete && extensionSupport && swapChainSupport;
}

VkResult 
pick_physical_device(VkInstance instance, VkSurfaceKHR surface) 
{
	physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (!deviceCount) {
		fputs("Devices: failed to find GPUs with Vulkan support.\n", stderr);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	VkPhysicalDevice devices[deviceCount];
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

	for (size_t i = 0; i < deviceCount; ++i) {
		if (is_device_suitable(devices[i], surface)) {
			physicalDevice = devices[i];
			break;
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			fputs("Devices: failed to find a suitable GPU.\n", stderr);
			return VK_ERROR_INITIALIZATION_FAILED;
		} 
	}

	return VK_SUCCESS;
}

VkResult 
create_logical_device() 
{
	logicalDevice = VK_NULL_HANDLE;

	uint32_t queues[] = {
		indices.graphicsFamily, 
		indices.presentFamily, 
	};
	uint32_t uniqueQueues = (queues[0] == queues[1]) ? 1 : 2;
	VkDeviceQueueCreateInfo queueCreateInfos[uniqueQueues];

	float queuePriority = 1.0f;
	for (size_t i = 0; i < uniqueQueues; ++i) {
		queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[i].pNext = nullptr;
		queueCreateInfos[i].flags = 0;
		queueCreateInfos[i].queueFamilyIndex = queues[i];
		queueCreateInfos[i].queueCount = 1;
		queueCreateInfos[i].pQueuePriorities = &queuePriority;
	}

	/* Create the logical device: */
	VkDeviceCreateInfo createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos;
	createInfo.queueCreateInfoCount = uniqueQueues;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(const char*);
	createInfo.ppEnabledExtensionNames = deviceExtensions;
	createInfo.ppEnabledLayerNames = nullptr;

	if (vkCreateDevice(physicalDevice, 
		    		&createInfo, 
		    		nullptr, 
		    		&logicalDevice) != VK_SUCCESS) {
		fputs("Devices: failed to create a logical device.\n", stderr);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily, 0, &presentQueue);

	return VK_SUCCESS;
}

VkSurfaceFormatKHR 
choose_swap_surface_format(void) 
{
	for (size_t i = 0; i < formats.count; ++i) {
		if (formats.data[i].format == VK_FORMAT_B8G8R8A8_SRGB && 
      			formats.data[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return formats.data[i];
		}
	}

	return formats.data[0];
}

VkPresentModeKHR 
choose_swap_present_mode(void) 
{
	for (size_t i = 0; i < presentModes.count; ++i) {
		if (presentModes.data[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentModes.data[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkResult 
create_swapChain(VkSurfaceKHR surface, uint32_t width, uint32_t height) 
{
	VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format();

	VkPresentModeKHR presentMode = choose_swap_present_mode();

	extent.width = width;
	extent.height = height;

	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily, 
		indices.presentFamily, 
	};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(logicalDevice, 
			  		&createInfo, 
			  		nullptr, 
			  		&swapChain) != VK_SUCCESS) {
		fputs("Devices: failed to create a valid swapchain.\n", stderr);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);

	images.data = (VkImage*) malloc(imageCount * sizeof(VkImage));
	images.count = imageCount;
	vkGetSwapchainImagesKHR(logicalDevice, 
			 	swapChain, 
			 	&imageCount, 
			 	images.data);

	swapChainFormat = surfaceFormat.format;

	return VK_SUCCESS;
}

VkResult 
create_image_views(void) 
{
	views.data = (VkImageView*) malloc(images.count * sizeof(VkImageView));
	views.count = images.count;

	for (size_t i = 0; i < images.count; ++i) {
		VkImageViewCreateInfo createInfo = { };
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images.data[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(logicalDevice, 
					&createInfo, 
					nullptr, 
					&views.data[i]) != VK_SUCCESS) {
			fputs("Devices: failed to create image views.\n", stderr);
			return VK_ERROR_INITIALIZATION_FAILED;
		}
	}


	return VK_SUCCESS;
}

VkResult 
create_render_pass(void) 
{
	VkAttachmentDescription colorAttachment = { };
	colorAttachment.format = swapChainFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = { };
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = { };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = { };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(logicalDevice, 
				&renderPassInfo, 
				nullptr, 
				&renderPass) != VK_SUCCESS) {
		fputs("Devices: failed to create a render pass.\n", stderr);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	return VK_SUCCESS;
}

VkResult 
create_framebuffers(void) 
{
	swapChainFramebuffers.data = malloc(views.count * sizeof(VkFramebuffer));
	if (!swapChainFramebuffers.data) { return VK_ERROR_INITIALIZATION_FAILED; }
	swapChainFramebuffers.count = views.count;

	for (size_t i = 0; i < swapChainFramebuffers.count; ++i) {
		VkImageView attachments[] = {
			views.data[i], 
		};

		VkFramebufferCreateInfo framebufferInfo = { };
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(logicalDevice, 
					  &framebufferInfo, 
					  nullptr, 
					  &swapChainFramebuffers.data[i]) != VK_SUCCESS) {
			fputs("Devices: failed to create a framebuffer.\n", stderr);
			free(swapChainFramebuffers.data);
			swapChainFramebuffers.count = 0;
			return VK_ERROR_INITIALIZATION_FAILED;
		}
	}

	return VK_SUCCESS;
}

VkResult 
create_command_pool(void) 
{
	VkCommandPoolCreateInfo poolInfo = { };
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = indices.graphicsFamily;

	if (vkCreateCommandPool(logicalDevice, 
				 &poolInfo, 
				 nullptr, 
				 &commandPool) != VK_SUCCESS) {
		fputs("Devices: failed to create command pool.\n", stderr);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	return VK_SUCCESS;
}

VkResult 
create_command_buffer(void) 
{
	VkCommandBufferAllocateInfo allocInfo = { };
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(logicalDevice, 
				      &allocInfo, 
				      &commandBuffer) != VK_SUCCESS) {
		fputs("Devices: failed to allocate command buffer.\n", stderr);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	return VK_SUCCESS;
}

VkResult 
record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) 
{
	VkCommandBufferBeginInfo beginInfo = { };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		fputs("Devices: failed to begin recording command buffers.\n", stderr);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	VkRenderPassBeginInfo renderPassInfo = { };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers.data[imageIndex];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = extent;

	VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) extent.width;
	viewport.height = (float) extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = extent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		fputs("Devices: failed to record command buffer.\n", stderr);
		return VK_ERROR_UNKNOWN;
	}

	return VK_SUCCESS;
}

VkResult 
create_sync_objects(void) 
{
	VkSemaphoreCreateInfo semaphoreInfo = { };
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = { };
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(logicalDevice, 
			       &semaphoreInfo, 
			       nullptr, 
			       &imageAvailableSph) != VK_SUCCESS || 
		vkCreateSemaphore(logicalDevice, 
			       &semaphoreInfo, 
			       nullptr, 
			       &renderFinishedSph) != VK_SUCCESS || 
		vkCreateFence(logicalDevice, 
				&fenceInfo, 
				nullptr, 
				&inFlightFence) != VK_SUCCESS) { 
		fputs("Devices: failed to create semaphores.\n", stderr);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	return VK_SUCCESS;
}

VkResult 
setup_devices(VkInstance instance, 
	      VkSurfaceKHR surface, 
	      uint32_t width, 
	      uint32_t height) 
{
	VkResult ret;

	ret = pick_physical_device(instance, surface);
	if (ret != VK_SUCCESS) { return ret; }

	ret = create_logical_device();
	if (ret != VK_SUCCESS) { return ret; }

	ret = create_swapChain(surface, width, height);
	if (ret != VK_SUCCESS) { return ret; }

	ret = create_image_views();
	if (ret != VK_SUCCESS) { return ret; }

	ret = create_render_pass();
	if (ret != VK_SUCCESS) { return ret; }

	ret = create_graphics_pipeline(logicalDevice, 
					extent, 
					renderPass, 
					&graphicsPipeline);
	if (ret != VK_SUCCESS) { return ret; }

	ret = create_framebuffers();
	if (ret != VK_SUCCESS) { return ret; }

	ret = create_command_pool();
	if (ret != VK_SUCCESS) { return ret; }

	ret = create_command_buffer();
	if (ret != VK_SUCCESS) { return ret; }

	ret = create_sync_objects();
	if (ret != VK_SUCCESS) { return ret; }

	return VK_SUCCESS;
}

VkResult 
draw_frame(void) 
{
	vkWaitForFences(logicalDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(logicalDevice, 1, &inFlightFence);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(logicalDevice, 
			       swapChain, 
			       UINT64_MAX, 
			       imageAvailableSph, 
			       VK_NULL_HANDLE, 
			       &imageIndex);

	vkResetCommandBuffer(commandBuffer, 0);
	record_command_buffer(commandBuffer, imageIndex);

	VkSubmitInfo submitInfo = { };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSph  };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkSemaphore signalSemaphores[] = { renderFinishedSph };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
		fputs("Devices: failed to submit draw command buffer.\n", stderr);
		return VK_ERROR_UNKNOWN;
	}

	VkPresentInfoKHR presentInfo = { };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(presentQueue, &presentInfo);

	return VK_SUCCESS;
}

void 
close_devices() 
{
	vkDeviceWaitIdle(logicalDevice);

	vkDestroySemaphore(logicalDevice, imageAvailableSph, nullptr);
	vkDestroySemaphore(logicalDevice, renderFinishedSph, nullptr);
	vkDestroyFence(logicalDevice, inFlightFence, nullptr);

	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

	for (size_t i = 0; i < swapChainFramebuffers.count; ++i) {
		vkDestroyFramebuffer(logicalDevice, swapChainFramebuffers.data[i], nullptr);
	}
	free(swapChainFramebuffers.data);
	swapChainFramebuffers.data = nullptr;
	swapChainFramebuffers.count = 0;

	close_graphics_pipeline(logicalDevice, graphicsPipeline);
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);

	if (views.count) {
		for (size_t i = 0; i < views.count; ++i) {
			vkDestroyImageView(logicalDevice, views.data[i], nullptr);
		}
		free(views.data);
		views.data = nullptr;
		views.count = 0;
	}

	if (images.count) {
		free(images.data);
		images.data = nullptr;
		images.count = 0;
	}
	vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);

	if (formats.count) {
		free(formats.data);
		formats.data = nullptr;
		formats.count = 0;
	}
	if (presentModes.count) {
		free(presentModes.data);
		formats.data = nullptr;
		presentModes.count = 0;
	}

	vkDestroyDevice(logicalDevice, nullptr);
}
