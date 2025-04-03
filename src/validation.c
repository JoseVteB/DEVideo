#include <stdio.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include "validation.h"

static VkDebugUtilsMessengerEXT debugMessenger;

bool 
check_validation_layers_support() 
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	VkLayerProperties availableLayers[layerCount];
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	size_t count = sizeof(validationLayers) / sizeof(const char*);
	for (size_t i = 0; i < count; ++i) {
		bool layerFound = false;

		for (size_t j = 0; j < layerCount; ++j) {
			if (strcmp(validationLayers[i], 
	      			availableLayers[j].layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) { return false; }
	}

	return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL 
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
	       VkDebugUtilsMessageTypeFlagsEXT messageType, 
	       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
	       void* pUserData) 
{
	FILE* output;
	char* severity;
	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: 
			output = stderr;
			severity = "[\033[0;30mERROR\033[0m]";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			output = stdout;
			severity = "[\033[0;34mINFO\033[0m]";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			output = stdout;
			severity = "[\033[0;32mVERBOSE\033[0m]";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			output = stderr;
			severity = "[\033[0;33mWARNING\033[0m]";
			break;
		default:
			output = stderr;
			severity = "[UNKNOWN]";
	}

	char* type;
	switch (messageType) {
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			type = "GENERAL";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			type = "VALIDATION";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			type = "PERFORMANCE";
			break;
		default:
			type = "UNKNOWN";
			break;
	}

	fprintf(output, "%s: %s %s\n", severity, type, pCallbackData->pMessage);

	return VK_FALSE;
}

VkResult 
CreateDebugUtilsMessengerEXT(VkInstance instance, 
			     const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
			     const VkAllocationCallbacks* pAllocator, 
			     VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) 
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) { 
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger); 
	} 

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void 
DestroyDebugUtilsMessengerEXT(VkInstance instance, 
			      VkDebugUtilsMessengerEXT debugMessenger, 
			      const VkAllocationCallbacks* pAllocator) 
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) 
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) { func(instance, debugMessenger, pAllocator); }
}

void 
populate_debugMessenger_createInfo(VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo) 
{
	pCreateInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	pCreateInfo->pNext = nullptr;
	pCreateInfo->flags = 0;
	pCreateInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
					| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
					| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	pCreateInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
					| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
					| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	pCreateInfo->pfnUserCallback = debug_callback;
}

VkResult 
setup_debug_messenger(VkInstance instance) 
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populate_debugMessenger_createInfo(&createInfo);

	return CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger);
}

void 
close_debug_messenger(VkInstance instance) 
{
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
}
