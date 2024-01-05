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

struct DeviceAndQueue {
	int Device = -1;
	int Queue = -1;
};

class VulkanPhysicalDevices {
public:
	VulkanPhysicalDevices() {}
	~VulkanPhysicalDevices() {}

	void Init(const VkInstance& inst, const VkSurfaceKHR& Surface);

	DeviceAndQueue SelectDevice(VkQueueFlags RequiredQueueType, bool SupportsPresent);

    std::vector<VkPhysicalDevice> m_devices;
    std::vector<VkPhysicalDeviceProperties> m_devProps;
    std::vector< std::vector<VkQueueFamilyProperties> > m_qFamilyProps;
    std::vector< std::vector<VkBool32> > m_qSupportsPresent;
    std::vector< std::vector<VkSurfaceFormatKHR> > m_surfaceFormats;
    std::vector<VkSurfaceCapabilitiesKHR> m_surfaceCaps;
    std::vector<VkPhysicalDeviceMemoryProperties> m_memProps;

private:
    void Allocate(int NumDevices);
};


void VulkanGetPhysicalDevices(const VkInstance& inst, const VkSurfaceKHR& Surface, VulkanPhysicalDevices& PhysDevices);

}