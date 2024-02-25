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

#include <vector>

#include <vulkan/vulkan.h>

namespace OgldevVK {

struct PhysicalDevice {
	VkPhysicalDevice m_physDevice;
	VkPhysicalDeviceProperties m_devProps;
	std::vector<VkQueueFamilyProperties> m_qFamilyProps;
	std::vector<VkBool32> m_qSupportsPresent;
	std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
	VkSurfaceCapabilitiesKHR m_surfaceCaps;
	VkPhysicalDeviceMemoryProperties m_memProps;
	std::vector<VkPresentModeKHR> m_presentModes;
};

class VulkanPhysicalDevices {
public:
	VulkanPhysicalDevices() {}
	~VulkanPhysicalDevices() {}

	void Init(const VkInstance& inst, const VkSurfaceKHR& Surface);

	u32 SelectDevice(VkQueueFlags RequiredQueueType, bool SupportsPresent);

	const PhysicalDevice& Selected() const;

private:
	std::vector<PhysicalDevice> m_devices;

	int m_devIndex = -1;
};

}