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

#ifdef VULKAN

#include <string>
#include <stdio.h>

#include "ogldev_vulkan.h"
#include "ogldev_util.h"


void VulkanPrintImageUsageFlags(const VkImageUsageFlags& flags)
{
    if (flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        printf("Image usage transfer src is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        printf("Image usage transfer dest is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_SAMPLED_BIT) {
        printf("Image usage sampled is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        printf("Image usage color attachment is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        printf("Image usage depth stencil attachment is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        printf("Image usage transient attachment is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
        printf("Image usage input attachment is supported\n");
    }    
}


void VulkanEnumExtProps(std::vector<VkExtensionProperties>& ExtProps)
{
    uint NumExt = 0;
    VkResult res = vkEnumerateInstanceExtensionProperties(NULL, &NumExt, NULL);
    CHECK_VULKAN_ERROR("vkEnumerateInstanceExtensionProperties error %d\n", res);
    
    printf("Found %d extensions\n", NumExt);
    
    ExtProps.resize(NumExt);

    res = vkEnumerateInstanceExtensionProperties(NULL, &NumExt, &ExtProps[0]);
    CHECK_VULKAN_ERROR("vkEnumerateInstanceExtensionProperties error %d\n", res);        
    
    for (uint i = 0 ; i < NumExt ; i++) {
        printf("Instance extension %d - %s\n", i, ExtProps[i].extensionName);
    }
}


VkShaderModule VulkanCreateShaderModule(VkDevice& device, const char* pFileName)
{
    int codeSize = 0;
    char* pShaderCode = ReadBinaryFile(pFileName, codeSize);
    assert(pShaderCode);
  
    VkShaderModuleCreateInfo shaderCreateInfo = {};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = codeSize;
    shaderCreateInfo.pCode = (const uint32_t*)pShaderCode;
    
    VkShaderModule shaderModule;
    VkResult res = vkCreateShaderModule(device, &shaderCreateInfo, NULL, &shaderModule);
    CHECK_VULKAN_ERROR("vkCreateShaderModule error %d\n", res);
    printf("Created shader %s\n", pFileName);
    return shaderModule;    
}


void VulkanGetPhysicalDevices(const VkInstance& inst, const VkSurfaceKHR& Surface, VulkanPhysicalDevices& PhysDevices)
{
    uint NumDevices = 0;
    
    VkResult res = vkEnumeratePhysicalDevices(inst, &NumDevices, NULL);
    CHECK_VULKAN_ERROR("vkEnumeratePhysicalDevices error %d\n", res);
    
    printf("Num physical devices %d\n", NumDevices);
    
    PhysDevices.m_devices.resize(NumDevices);
    PhysDevices.m_devProps.resize(NumDevices);
    PhysDevices.m_qFamilyProps.resize(NumDevices);
    PhysDevices.m_qSupportsPresent.resize(NumDevices);
    PhysDevices.m_surfaceFormats.resize(NumDevices);
    PhysDevices.m_surfaceCaps.resize(NumDevices);
        
    res = vkEnumeratePhysicalDevices(inst, &NumDevices, &PhysDevices.m_devices[0]);
    CHECK_VULKAN_ERROR("vkEnumeratePhysicalDevices error %d\n", res);
    
    for (uint i = 0 ; i < NumDevices ; i++) {
        const VkPhysicalDevice& PhysDev = PhysDevices.m_devices[i];
        vkGetPhysicalDeviceProperties(PhysDev, &PhysDevices.m_devProps[i]);
        
        printf("Device name: %s\n", PhysDevices.m_devProps[i].deviceName);
        uint32_t apiVer = PhysDevices.m_devProps[i].apiVersion;
        printf("    API version: %d.%d.%d\n", VK_VERSION_MAJOR(apiVer),
                                          VK_VERSION_MINOR(apiVer),
                                          VK_VERSION_PATCH(apiVer));
        uint NumQFamily = 0;         
        
        vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, NULL);
    
        printf("    Num of family queues: %d\n", NumQFamily);

        PhysDevices.m_qFamilyProps[i].resize(NumQFamily);
        PhysDevices.m_qSupportsPresent[i].resize(NumQFamily);

        vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, &(PhysDevices.m_qFamilyProps[i][0]));
        
        for (uint q = 0 ; q < NumQFamily ; q++) {
            res = vkGetPhysicalDeviceSurfaceSupportKHR(PhysDev, q, Surface, &(PhysDevices.m_qSupportsPresent[i][q]));
            CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfaceSupportKHR error %d\n", res);
        }
              
        uint NumFormats = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, NULL);
        assert(NumFormats > 0);
        
        PhysDevices.m_surfaceFormats[i].resize(NumFormats);
        
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, &(PhysDevices.m_surfaceFormats[i][0]));
        CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR error %d\n", res);
    
        for (uint j = 0 ; j < NumFormats ; j++) {
            const VkSurfaceFormatKHR& SurfaceFormat = PhysDevices.m_surfaceFormats[i][j];
            printf("    Format %d color space %d\n", SurfaceFormat.format , SurfaceFormat.colorSpace);
        }
        
        res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysDev, Surface, &(PhysDevices.m_surfaceCaps[i]));
        CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfaceCapabilitiesKHR error %d\n", res);
       
        VulkanPrintImageUsageFlags(PhysDevices.m_surfaceCaps[i].supportedUsageFlags);
  
        uint NumPresentModes = 0;
    
        res = vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDev, Surface, &NumPresentModes, NULL);
        CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfacePresentModesKHR error %d\n", res);

        assert(NumPresentModes != 0);

        printf("Number of presentation modes %d\n", NumPresentModes);
    }
}

#endif