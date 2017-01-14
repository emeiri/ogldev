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

#include "ogldev_util.h"
#include "ogldev_vulkan_core.h"

PFN_vkCreateDebugReportCallbackEXT my_vkCreateDebugReportCallbackEXT = NULL; 

       
VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(
    VkDebugReportFlagsEXT       flags,
    VkDebugReportObjectTypeEXT  objectType,
    uint64_t                    object,
    size_t                      location,
    int32_t                     messageCode,
    const char*                 pLayerPrefix,
    const char*                 pMessage,
    void*                       pUserData)
{
    printf("%s\n", pMessage);
    return VK_FALSE;
}




OgldevVulkanCore::OgldevVulkanCore(const char* pAppName) 
{
    m_appName = std::string(pAppName);
    m_gfxDevIndex = -1;
    m_gfxQueueFamily = -1;
}


OgldevVulkanCore::~OgldevVulkanCore()
{
    
}


void OgldevVulkanCore::Init(VulkanWindowControl* pWindowControl)
{ 
    std::vector<VkExtensionProperties> ExtProps;
    VulkanEnumExtProps(ExtProps);

    CreateInstance();
    
    m_surface = pWindowControl->CreateSurface(m_inst);
    assert(m_surface);          

    printf("Surface created\n");

    VulkanGetPhysicalDevices(m_inst, m_surface, m_physDevices);
    SelectPhysicalDevice();
    CreateLogicalDevice();
}

const VkPhysicalDevice& OgldevVulkanCore::GetPhysDevice() const
{
    assert(m_gfxDevIndex >= 0);
    return m_physDevices.m_devices[m_gfxDevIndex];
}

const VkSurfaceFormatKHR& OgldevVulkanCore::GetSurfaceFormat() const
{
    assert(m_gfxDevIndex >= 0);
    return m_physDevices.m_surfaceFormats[m_gfxDevIndex][0];
}


const VkSurfaceCapabilitiesKHR OgldevVulkanCore::GetSurfaceCaps() const
{
    assert(m_gfxDevIndex >= 0);
    return m_physDevices.m_surfaceCaps[m_gfxDevIndex];
    
}


void OgldevVulkanCore::SelectPhysicalDevice()
{
    for (uint i = 0 ; i < m_physDevices.m_devices.size() ; i++) {
                
        for (uint j = 0 ; j < m_physDevices.m_qFamilyProps[i].size() ; j++) {
            VkQueueFamilyProperties& QFamilyProp = m_physDevices.m_qFamilyProps[i][j];
            
            printf("Family %d Num queues: %d\n", j, QFamilyProp.queueCount);
            VkQueueFlags flags = QFamilyProp.queueFlags;
            printf("    GFX %s, Compute %s, Transfer %s, Sparse binding %s\n",
                    (flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
                    (flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
                    (flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
                    (flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No");
            
            if ((flags & VK_QUEUE_GRAPHICS_BIT) && (m_gfxDevIndex == -1)) {    
                if (!m_physDevices.m_qSupportsPresent[i][j]) {
                    printf("Present is not supported\n");
                    continue;
                }

                m_gfxDevIndex = i;
                m_gfxQueueFamily = j;
                printf("Using GFX device %d and queue family %d\n", m_gfxDevIndex, m_gfxQueueFamily);
            }
        }
    }
    
    if (m_gfxDevIndex == -1) {
        printf("No GFX device found!\n");
        assert(0);
    }    
}


void OgldevVulkanCore::CreateInstance()
{
    VkApplicationInfo appInfo = {};       
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_appName.c_str();
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    const char* pInstExt[] = {
#ifdef ENABLE_DEBUG_LAYERS
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif        
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32    
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else    
        VK_KHR_XCB_SURFACE_EXTENSION_NAME
#endif            
    };
    
#ifdef ENABLE_DEBUG_LAYERS    
    const char* pInstLayers[] = {
        "VK_LAYER_LUNARG_standard_validation"
    };
#endif    
    
    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;
#ifdef ENABLE_DEBUG_LAYERS    
    instInfo.enabledLayerCount = ARRAY_SIZE_IN_ELEMENTS(pInstLayers);
    instInfo.ppEnabledLayerNames = pInstLayers;
#endif    
    instInfo.enabledExtensionCount = ARRAY_SIZE_IN_ELEMENTS(pInstExt);
    instInfo.ppEnabledExtensionNames = pInstExt;         

    VkResult res = vkCreateInstance(&instInfo, NULL, &m_inst);
    CHECK_VULKAN_ERROR("vkCreateInstance %d\n", res);
        
#ifdef ENABLE_DEBUG_LAYERS
    // Get the address to the vkCreateDebugReportCallbackEXT function
    my_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_inst, "vkCreateDebugReportCallbackEXT"));
    
    // Register the debug callback
    VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
    callbackCreateInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    callbackCreateInfo.pNext       = NULL;
    callbackCreateInfo.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                     VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    callbackCreateInfo.pfnCallback = &MyDebugReportCallback;
    callbackCreateInfo.pUserData   = NULL;

    VkDebugReportCallbackEXT callback;
    res = my_vkCreateDebugReportCallbackEXT(m_inst, &callbackCreateInfo, NULL, &callback);
    CHECK_VULKAN_ERROR("my_vkCreateDebugReportCallbackEXT error %d\n", res);
#endif    
}


void OgldevVulkanCore::CreateLogicalDevice()
{
    VkDeviceQueueCreateInfo qInfo = {};
    qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    
    float qPriorities = 1.0f;
    qInfo.queueCount = 1;
    qInfo.pQueuePriorities = &qPriorities;
    qInfo.queueFamilyIndex = m_gfxQueueFamily;

    const char* pDevExt[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
       
    VkDeviceCreateInfo devInfo = {};
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.enabledExtensionCount = ARRAY_SIZE_IN_ELEMENTS(pDevExt);
    devInfo.ppEnabledExtensionNames = pDevExt;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &qInfo;
       
    VkResult res = vkCreateDevice(GetPhysDevice(), &devInfo, NULL, &m_device);

    CHECK_VULKAN_ERROR("vkCreateDevice error %d\n", res);
   
    printf("Device created\n");
}

#endif