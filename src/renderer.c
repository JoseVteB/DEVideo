#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "validation.h"

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

VkInstance instance;
static const char* instanceExtensions[] = {
	"VK_KHR_surface", 
	"VK_KHR_wayland_surface", 
	"VK_EXT_debug_utils", 
};

int 
create_instance(const char* appName) 
{
	if (enableValidationLayers && !check_validation_layers_support()) {
		fputs("Renderer: validation layers requested, but not available!\n", stderr);
		return EXIT_FAILURE;
	}

	VkApplicationInfo appInfo = { };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = sizeof(instanceExtensions) / sizeof(const char*);
	createInfo.ppEnabledExtensionNames = instanceExtensions;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = sizeof(validationLayers) / sizeof(const char*);
		createInfo.ppEnabledLayerNames = validationLayers;

		populate_debugMessenger_createInfo(&debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		fputs("Renderer: failed to create a Vulkan instance.\n", stderr);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int 
init_renderer(const char* appName) 
{
	create_instance(appName);
	if (enableValidationLayers) { setup_debug_messenger(instance); }

	return EXIT_SUCCESS;
}

void 
close_renderer() 
{
	if (enableValidationLayers) { close_debug_messenger(instance); }
	vkDestroyInstance(instance, nullptr);
}
