#ifndef	VALIDATION_H
#define	VALIDATION_H

#include <vulkan/vulkan.h>

static const char* validationLayers[] = {
	"VK_LAYER_KHRONOS_validation", 
};

bool 
check_validation_layers_support(void);

void 
populate_debugMessenger_createInfo(VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo);

VkResult 
setup_debug_messenger(VkInstance instance);

void 
close_debug_messenger(VkInstance instance);

#endif	/* VALIDATION_H */
