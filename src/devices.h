#ifndef	DEVICES_H
#define	DEVICES_H

#include <vulkan/vulkan.h>

VkResult 
setup_devices(VkInstance instance, 
	      VkSurfaceKHR surface, 
	      uint32_t width, 
	      uint32_t height);

VkResult 
draw_frame(void);

void 
close_devices();

#endif	/* DEVICES_H */
