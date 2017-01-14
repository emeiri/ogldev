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

#ifdef VULKAN

#include <vector>

//#define ENABLE_DEBUG_LAYERS

#ifdef _WIN32
#define  VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"
#include "vulkan/vk_sdk_platform.h"
#else
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
#endif

#include "ogldev_util.h"


#define CHECK_VULKAN_ERROR(msg, res)    \
    if (res != VK_SUCCESS) {            \
        OGLDEV_ERROR(msg, res);         \
        abort();                        \
    }

struct VulkanPhysicalDevices {
    std::vector<VkPhysicalDevice> m_devices;
    std::vector<VkPhysicalDeviceProperties> m_devProps;
    std::vector< std::vector<VkQueueFamilyProperties> > m_qFamilyProps;
    std::vector< std::vector<VkBool32> > m_qSupportsPresent;
    std::vector< std::vector<VkSurfaceFormatKHR> > m_surfaceFormats;
    std::vector<VkSurfaceCapabilitiesKHR> m_surfaceCaps;
};

void VulkanEnumExtProps(std::vector<VkExtensionProperties>& ExtProps);
void VulkanPrintImageUsageFlags(const VkImageUsageFlags& flags);
VkShaderModule VulkanCreateShaderModule(VkDevice& device, const char* pFileName);
void VulkanGetPhysicalDevices(const VkInstance& inst, const VkSurfaceKHR& Surface, VulkanPhysicalDevices& PhysDevices);

class VulkanWindowControl
{
protected:
    VulkanWindowControl() {};

    ~VulkanWindowControl() {};

public:

    virtual void Init(uint Width, uint Height) = 0;
    
    virtual VkSurfaceKHR CreateSurface(VkInstance& inst) = 0;
};

#endif // VULKAN

#endif
