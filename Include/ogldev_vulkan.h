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

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>



#define CheckVulkanError(msg)           \
    if (res != VK_SUCCESS) {            \
        printf(msg);                    \
        printf(", result: %x\n", res);  \
        abort();                        \
    }

bool VulkanEnumExtProps(std::vector<VkExtensionProperties>& ExtProps);
void VulkanPrintImageUsageFlags(const VkImageUsageFlags& flags);


#endif