/*
		Copyright 2024 Etay Meiri

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <vector>
#include <assert.h>

#include "vulkan_core.h"

namespace OgldevVK {

VulkanCore::VulkanCore(GLFWwindow* pWindow)
{
	m_pWindow = pWindow;
}


VulkanCore::~VulkanCore()
{

}


void VulkanCore::Init(const char* pAppName)
{
	CreateInstance(pAppName);
	InitDebugCallbacks();
	CreateSurface();
	m_physDevices.Init(m_instance, m_surface);
	m_devAndQueue = m_physDevices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
	CreateDevice();
	CreateSwapChain();
}

void VulkanCore::CreateInstance(const char* pAppName)
{
	std::vector<const char*> ValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	std::vector<const char*> Extensions = {		
		"VK_KHR_surface",
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
		"VK_KHR_win32_surface",
#endif
#if defined (__APPLE__)
		"VK_MVK_macos_surface",
#endif
#if defined (__linux__)
		"VK_KHR_xcb_surface",
#endif
	    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
	};

	const VkApplicationInfo AppInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = pAppName,
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Ogldev Vulkan Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_1
	};

	VkInstanceCreateInfo CreateInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pApplicationInfo = &AppInfo,
		.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size()),
		.ppEnabledLayerNames = ValidationLayers.data(),
		.enabledExtensionCount = (uint32_t)(Extensions.size()),
		.ppEnabledExtensionNames = Extensions.data()
	};

	VkResult res = vkCreateInstance(&CreateInfo, NULL, &m_instance);
	CHECK_VK_RESULT(res, "Create instance");
	printf("Vulkan instance created\n");
}


static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
												    VkDebugUtilsMessageTypeFlagsEXT Type,
													const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
													void* pUserData)
{
	printf("Validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}


static VKAPI_ATTR VkBool32 VKAPI_CALL ReportCallback(VkDebugReportFlagsEXT      flags,
													 VkDebugReportObjectTypeEXT objectType,
													 uint64_t                   object,
													 size_t                     location,
													 int32_t                    messageCode,
													 const char* pLayerPrefix,
													 const char* pMessage,
													 void* PUserData)
{
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		return VK_FALSE;
	}

	printf("Debug callback (%s): %s\n", pLayerPrefix, pMessage);
	return VK_FALSE;
}


void VulkanCore::InitDebugCallbacks()
{
	VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = &DebugCallback,
		.pUserData = nullptr
	};

	PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessenger = VK_NULL_HANDLE;
	CreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");

	VkResult res = CreateDebugUtilsMessenger(m_instance, &MessengerCreateInfo, NULL, &m_messenger);
	CHECK_VK_RESULT(res, "Create debug utils messenger");

	const VkDebugReportCallbackCreateInfoEXT CallbackCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
		.pNext = NULL,
		.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
 				 VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
				 VK_DEBUG_REPORT_ERROR_BIT_EXT |
				 VK_DEBUG_REPORT_DEBUG_BIT_EXT,
		.pfnCallback = &ReportCallback,
		.pUserData = NULL
	};

	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
	CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT");

	res = CreateDebugReportCallback(m_instance, &CallbackCreateInfo, NULL, &m_reportCallback);
	CHECK_VK_RESULT(res, "Create debug report callback");

	printf("Debug callbacks initialized\n");
}


void VulkanCore::CreateSurface()
{
	if (glfwCreateWindowSurface(m_instance, m_pWindow, NULL, &m_surface)) {
		fprintf(stderr, "Error creating GLFW window surface\n");
		exit(1);
	}

	printf("GLFW window surface created\n");
}


void VulkanCore::CreateDevice()
{
	VkDeviceQueueCreateInfo qInfo = {};
	qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

	float qPriorities = 1.0f;
	qInfo.queueCount = 1;
	qInfo.pQueuePriorities = &qPriorities;
	qInfo.queueFamilyIndex = m_devAndQueue.Queue;

	std::vector<const char*> DevExts = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo devInfo = {};
	devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devInfo.enabledExtensionCount = (uint32_t)DevExts.size();
	devInfo.ppEnabledExtensionNames = DevExts.data();
	devInfo.queueCreateInfoCount = 1;
	devInfo.pQueueCreateInfos = &qInfo;

	VkResult res = vkCreateDevice(m_physDevices.m_devices[m_devAndQueue.Device], &devInfo, NULL, &m_device);
	CHECK_VK_RESULT(res, "Create device\n");

	printf("Device created\n");
}


void VulkanCore::CreateSwapChain()
{
	const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_physDevices.m_surfaceCaps[m_devAndQueue.Device];

	assert(SurfaceCaps.currentExtent.width != -1);

	uint NumImages = 2;

	assert(NumImages >= SurfaceCaps.minImageCount);
	assert(NumImages <= SurfaceCaps.maxImageCount);

	VkSwapchainCreateInfoKHR SwapChainCreateInfo = {};

	SwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapChainCreateInfo.surface = m_surface;
	SwapChainCreateInfo.minImageCount = NumImages;
	SwapChainCreateInfo.imageFormat = m_physDevices.m_surfaceFormats[m_devAndQueue.Device][0].format;
	SwapChainCreateInfo.imageColorSpace = m_physDevices.m_surfaceFormats[m_devAndQueue.Device][0].colorSpace;
	SwapChainCreateInfo.imageExtent = SurfaceCaps.currentExtent;
	SwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	SwapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	SwapChainCreateInfo.imageArrayLayers = 1;
	SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	SwapChainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	SwapChainCreateInfo.clipped = true;
	SwapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	VkResult res = vkCreateSwapchainKHR(m_device, &SwapChainCreateInfo, NULL, &m_swapChain);
	CHECK_VK_RESULT(res, "vkCreateSwapchainKHR\n");

	printf("Swap chain created\n");

	uint NumSwapChainImages = 0;
	res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, NULL);
	CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");
	assert(NumImages == NumSwapChainImages);

	printf("Number of images %d\n", NumSwapChainImages);

	m_images.resize(NumSwapChainImages);
	m_cmdBufs.resize(NumSwapChainImages);

	res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, &(m_images[0]));
	CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");
}

}