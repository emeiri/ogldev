/*

	Copyright 2016 Etay Meiri

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

#ifndef OGLDEV_VULKAN_H
#define OGLDEV_VULKAN_H

#include <vector>

#ifdef _WIN32
#include "c:\\VulkanSDK\\1.0.21.1\\Include\\vulkan\\vulkan.h"
#include "c:\\VulkanSDK\\1.0.21.1\\Include\\vulkan\\vk_sdk_platform.h"
#else
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
#endif


#define CheckVulkanError(msg)           \
    if (res != VK_SUCCESS) {            \
        printf(msg);                    \
        printf(", result: %x\n", res);  \
        abort();                        \
    }

struct VulkanPhysicalDevices {
    std::vector<VkPhysicalDevice> m_devices;
    std::vector<VkPhysicalDeviceProperties> m_devProps;
    std::vector<std::vector<VkQueueFamilyProperties> > m_qFamilyProps;
};

bool VulkanEnumExtProps(std::vector<VkExtensionProperties>& ExtProps);
void VulkanPrintImageUsageFlags(const VkImageUsageFlags& flags);
VkShaderModule VulkanCreateShaderModule(VkDevice& device, const char* pFileName);
bool VulkanGetPhysicalDevices(const VkInstance& inst, VulkanPhysicalDevices& PhysDevices);

class VulkanWindowControl
{
protected:
    VulkanWindowControl() {};

    ~VulkanWindowControl() {};

public:

    virtual bool Init(uint Width, uint Height) = 0;
    
    virtual VkSurfaceKHR CreateSurface(VkInstance& inst) = 0;

    virtual void PreRun() = 0;

    virtual bool PollEvent() = 0;
};

#endif