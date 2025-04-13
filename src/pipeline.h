#ifndef	PIPELINE_H
#define	PIPELINE_H

#include <vulkan/vulkan.h>

int 
create_graphics_pipeline(VkDevice device, 
			 VkExtent2D extent, 
			 VkRenderPass renderPass, 
			 VkPipeline* pGraphicsPipeline);

void 
close_graphics_pipeline(VkDevice device, VkPipeline graphicsPipeline);

#endif	/* PIPELINE_H */
