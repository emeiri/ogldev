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

#include "ogldev_types.h"
#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_util.h"

namespace OgldevVK {

VulkanCore::VulkanCore()
{
}


VulkanCore::~VulkanCore()
{
	vkDestroyInstance(m_instance, NULL);
	printf("Vulkan instance destroyed\n");
}


void VulkanCore::Init(const char* pAppName)
{
	CreateInstance(pAppName);
}


void VulkanCore::CreateInstance(const char* pAppName)
{
	std::vector<const char*> Layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	std::vector<const char*> Extensions = {		
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

	VkApplicationInfo AppInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = pAppName,
		.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		.pEngineName = "Ogldev Vulkan Tutorials",
		.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};

	VkInstanceCreateInfo CreateInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,				// reserved for future use. Must be zero
		.pApplicationInfo = &AppInfo,
		.enabledLayerCount = (u32)(Layers.size()),
		.ppEnabledLayerNames = Layers.data(),
		.enabledExtensionCount = (u32)(Extensions.size()),
		.ppEnabledExtensionNames = Extensions.data()
	};

	VkResult res = vkCreateInstance(&CreateInfo, NULL, &m_instance);
	CHECK_VK_RESULT(res, "Create instance");
	printf("Vulkan instance created\n");
}



}
