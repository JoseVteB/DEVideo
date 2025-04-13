#include <stdio.h>
#include <stdlib.h>

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#include <wayland-client.h>

#include "devices.h"
#include "renderer.h"
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

VkSurfaceKHR surface;

VkResult 
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
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	return VK_SUCCESS;
}

VkResult 
create_surface(VkInstance instance, 
	       struct wl_display* pDisplay, 
	       struct wl_surface* pSurface) 
{
	VkWaylandSurfaceCreateInfoKHR createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	createInfo.display = pDisplay;
	createInfo.surface = pSurface;

	if (vkCreateWaylandSurfaceKHR(instance, 
				       &createInfo, 
				       nullptr, 
				       &surface) != VK_SUCCESS) {
		fputs("Renderer: failed to create the main vulkan surface.\n", stderr);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	return VK_SUCCESS;
}

int 
init_renderer(const char* appName, 
	      struct wl_display* pDisplay, 
	      struct wl_surface* pSurface) 
{
	if (create_instance(appName) != VK_SUCCESS) { return EXIT_FAILURE; }
	if (enableValidationLayers) { setup_debug_messenger(instance); }

	if (create_surface(instance, pDisplay, pSurface) != VK_SUCCESS) { 
		return EXIT_FAILURE; 
	}
	if (setup_devices(instance, surface, 800, 600) != VK_SUCCESS) { return EXIT_FAILURE; }

	return EXIT_SUCCESS;
}

void 
render_surface(void) 
{
	draw_frame();
}

void 
close_renderer(void) 
{
	close_devices();
	vkDestroySurfaceKHR(instance, surface, nullptr);

	if (enableValidationLayers) { close_debug_messenger(instance); }
	vkDestroyInstance(instance, nullptr);
}
