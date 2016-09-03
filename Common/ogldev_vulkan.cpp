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
