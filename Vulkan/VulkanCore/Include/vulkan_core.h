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

#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_utils.h"
#include "vk_device.h"

namespace OgldevVK {

class VulkanCore {
public:
	VulkanCore(GLFWwindow* pWindow);
	~VulkanCore();

	void Init(const char* pAppName);

	int GetQueue() const { return m_devAndQueue.Queue; }

	VkDevice GetDevice() const { return m_device; }

	int GetNumImages() const { return (int)m_images.size(); }

	std::vector<VkImage>& GetImages() { return m_images;  }

private:

	void CreateInstance(const char* pAppName);
	void InitDebugCallbacks();
	void CreateSurface();
	void CreateDevice();
	void CreateSwapChain();

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_messenger;
	VkDebugReportCallbackEXT m_reportCallback;
	GLFWwindow* m_pWindow = NULL;
	VkSurfaceKHR m_surface;
	VulkanPhysicalDevices m_physDevices;
	DeviceAndQueue m_devAndQueue;
	VkDevice m_device;
	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_images;	
};
}