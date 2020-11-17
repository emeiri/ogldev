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

#ifndef OGLDEV_VULKAN_CORE_H
#define OGLDEV_VULKAN_CORE_H

#include <string>
#include <stdio.h>
#include <assert.h>

#include "ogldev_vulkan.h"

class OgldevVulkanCore
{
public:
    OgldevVulkanCore(const char* pAppName);
    ~OgldevVulkanCore();
    
    void Init(VulkanWindowControl* pWindowControl);
              
    const VkPhysicalDevice& GetPhysDevice() const;
    
    const VkSurfaceFormatKHR& GetSurfaceFormat() const;
    
    const VkSurfaceCapabilitiesKHR GetSurfaceCaps() const;
    
    const VkSurfaceKHR& GetSurface() const { return m_surface; }
    
    int GetQueueFamily() const { return m_gfxQueueFamily; }
    
    VkInstance& GetInstance() { return m_inst; }
    
    VkDevice& GetDevice() { return m_device; }
    
    u32 GetMemoryTypeIndex(u32 memTypeBits, VkMemoryPropertyFlags memPropFlags);
    
    VkSemaphore CreateSemaphore();
    
private:
    void CreateInstance();
    void CreateSurface();
    void SelectPhysicalDevice();
    void CreateLogicalDevice();

    // Vulkan objects
    VkInstance m_inst;
    VkSurfaceKHR m_surface;
    VulkanPhysicalDevices m_physDevices;
    VkDevice m_device;
    
    // Internal stuff
    std::string m_appName;
    int m_gfxDevIndex;
    int m_gfxQueueFamily;
};


#endif
