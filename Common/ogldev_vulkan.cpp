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


bool VulkanEnumExtProps(std::vector<VkExtensionProperties>& ExtProps)
{
    uint NumExt = 0;
    VkResult res = vkEnumerateInstanceExtensionProperties(NULL, &NumExt, NULL);
    
    if (res != VK_SUCCESS) {
        printf("Error enumerating extensions: %x\n", res);
        return false;
    }
    
    printf("Found %d extensions\n", NumExt);
    
    ExtProps.resize(NumExt);

    res = vkEnumerateInstanceExtensionProperties(NULL, &NumExt, &ExtProps[0]);
    
    if (res != VK_SUCCESS) {
        printf("Error enumerating extensions: %x\n", res);
        return false;
    }
        
    for (uint i = 0 ; i < NumExt ; i++) {
        printf("Instance extension %d - %s\n", i, ExtProps[i].extensionName);
    }
    
    return true;
}


VkShaderModule VulkanCreateShaderModule(VkDevice& device, const char* pFileName)
{
   /* std::vector<int> s;
    
    if (!ReadBinaryFile(pFileName, s)) {
        return NULL;
    }*/
    
    std::string s;
    
    if (!ReadFile(pFileName, s)) {
        return NULL;
    }
  
    VkShaderModuleCreateInfo shaderCreateInfo = {};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = s.size();
    shaderCreateInfo.pCode = (const uint32_t*)s.c_str();
    
    VkShaderModule shaderModule;
    VkResult res = vkCreateShaderModule(device, &shaderCreateInfo, NULL, &shaderModule);
    CheckVulkanError("vkCreateShaderModule failed");
    printf("Created shader %s\n", pFileName);
    return shaderModule;    
}




bool VulkanGetPhysicalDevices(const VkInstance& inst, VulkanPhysicalDevices& PhysDevices)
{
    uint NumDevices = 0;
    
    VkResult res = vkEnumeratePhysicalDevices(inst, &NumDevices, NULL);
    
    if (res != VK_SUCCESS) {
        OGLDEV_ERROR("vkEnumeratePhysicalDevices error");
    }
    
    printf("Num physical devices %d\n", NumDevices);
    
    PhysDevices.m_devices.resize(NumDevices);
    PhysDevices.m_devProps.resize(NumDevices);
    PhysDevices.m_qFamilyProps.resize(NumDevices);
    
    res = vkEnumeratePhysicalDevices(inst, &NumDevices, &PhysDevices.m_devices[0]);
    
    if (res != VK_SUCCESS) {
        OGLDEV_ERROR("vkEnumeratePhysicalDevices");
    }
  
    for (uint i = 0 ; i < NumDevices ; i++) {
        vkGetPhysicalDeviceProperties(PhysDevices.m_devices[i], &PhysDevices.m_devProps[i]);
        
        printf("Device name: %s\n", PhysDevices.m_devProps[i].deviceName);
        uint32_t apiVer = PhysDevices.m_devProps[i].apiVersion;
        printf("API version: %d.%d.%d\n", VK_VERSION_MAJOR(apiVer),
                                          VK_VERSION_MINOR(apiVer),
                                          VK_VERSION_PATCH(apiVer));
        uint NumQFamily = 0;         
        
        vkGetPhysicalDeviceQueueFamilyProperties(PhysDevices.m_devices[i], &NumQFamily, NULL);
    
        printf("Num of family queues: %d\n", NumQFamily);

        PhysDevices.m_qFamilyProps[i].resize(NumQFamily);

        vkGetPhysicalDeviceQueueFamilyProperties(PhysDevices.m_devices[i], &NumQFamily, &(PhysDevices.m_qFamilyProps[i][0]));
                
      /*  for (uint j = 0 ; j < NumQFamily ; j++) {
            VkQueueFamilyProperties& QFamilyProp = PhysDevices.m_qFamilyProps[i][j];
            
            printf("Family %d Num queues: %d\n", j, QFamilyProp.queueCount);
            VkQueueFlags flags = QFamilyProp.queueFlags;
            printf("    GFX %s, Compute %s, Transfer %s, Sparse binding %s\n",
                    (flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
                    (flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
                    (flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
                    (flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No");
            
            if ((flags & VK_QUEUE_GRAPHICS_BIT) && (m_gfxDevIndex == -1)) {
                m_gfxDevIndex = i;
                m_gfxQueueFamily = j;
                printf("Using GFX device %d and queue family %d\n", m_gfxDevIndex, m_gfxQueueFamily);
            }
        }*/
    }
    
    /*if (m_gfxDevIndex == -1) {
        printf("No GFX device found!\n");
        assert(0);
    }    
        */
    return true;
}
