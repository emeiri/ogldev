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

    Tutorial 51 - Vulkan Clear Window
*/

#include <cfloat>
#include <math.h>
#include <GL/glew.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/types.h>
#include <vector>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_vulkan_core.h"

#include "ogldev_xcb_control.h"

#define WINDOW_WIDTH  1024  
#define WINDOW_HEIGHT 1024

class OgldevVulkanApp
{
public:

    OgldevVulkanApp(const char* pAppName);
    
    ~OgldevVulkanApp();
    
    void Init();    
    
    void Run();
    
private:

    void EnumPhysDeviceExtProps();
    void CreateSwapChain();
    void CreateCommandBuffer();
    void CreateSemaphore();
    void RecordCommandBuffers();
    void Draw();

    VulkanWindowControl* m_pWindowControl;
    OgldevVulkanCore m_core;    
    std::vector<VkImage> m_images;
    VkSwapchainKHR m_swapChainKHR;
    VkQueue m_queue;
    std::vector<VkCommandBuffer> m_presentQCmdBuffs;
    VkCommandPool m_presentQCmdPool;
    uint m_frameCount;
};


OgldevVulkanApp::OgldevVulkanApp(const char* pAppName) : m_core(pAppName)
{
    m_frameCount = 0;
}


OgldevVulkanApp::~OgldevVulkanApp()
{
    
}

    
void OgldevVulkanApp::EnumPhysDeviceExtProps()
{
   /* uint NumExt = 0;
    
    VkPhysicalDevice& gfxPhysDev = m_physDevices[m_gfxDevIndex];
    
    VkResult res = vkEnumerateDeviceExtensionProperties(gfxPhysDev, NULL, &NumExt, NULL);
    
    if (res != VK_SUCCESS) {
        printf("Error enumerating device extensions %x\n", res);
        assert(0);
    }
    
    std::vector<VkExtensionProperties> ExtProps(NumExt);

    res = vkEnumerateDeviceExtensionProperties(gfxPhysDev, NULL, &NumExt, &ExtProps[0]);
    
    if (res != VK_SUCCESS) {
        printf("Error enumerating extensions");
        assert(0);
    }
        
    for (uint i = 0 ; i < NumExt ; i++) {
        printf("Device extension %d - %s\n", i, ExtProps[i].extensionName);
        m_devExt.push_back(std::string(ExtProps[i].extensionName));
    }     */       
    
    //vkGetPhysicalDeviceMemoryProperties(m_physDevices[m_gfxDevIndex], &m_memProps);
}


void OgldevVulkanApp::CreateSwapChain()
{          
    assert(m_core.GetSurfaceFormat().format != VK_FORMAT_UNDEFINED);
    
    const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_core.GetSurfaceCaps();
         
    VkExtent2D SwapChainExtent = SurfaceCaps.currentExtent;
    
    if (SurfaceCaps.currentExtent.width == -1) {
        SwapChainExtent.width = WINDOW_WIDTH;
        SwapChainExtent.height = WINDOW_HEIGHT;
    }
    else {
        SwapChainExtent = SurfaceCaps.currentExtent;
    }
    
    printf("Swap chain extent: width %d height %d\n", SwapChainExtent.width, SwapChainExtent.height);
    
    uint NumImages = SurfaceCaps.minImageCount + 1;
    
    if ((SurfaceCaps.maxImageCount > 0) &&
        (NumImages > SurfaceCaps.maxImageCount)) {
        NumImages =  SurfaceCaps.maxImageCount;
    }
    
    printf("Num images: %d\n", NumImages);
    
    VkSurfaceTransformFlagBitsKHR preTransform;
    
    if (SurfaceCaps.currentTransform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else {
        preTransform = SurfaceCaps.currentTransform;
    }
    
    VkSwapchainCreateInfoKHR SwapChainCreateInfo = {};
    
    SwapChainCreateInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapChainCreateInfo.surface          = m_core.GetSurface();
    SwapChainCreateInfo.minImageCount    = NumImages;
    SwapChainCreateInfo.imageFormat      = m_core.GetSurfaceFormat().format;
    SwapChainCreateInfo.imageColorSpace  = m_core.GetSurfaceFormat().colorSpace;
    SwapChainCreateInfo.imageExtent      = SwapChainExtent;
    SwapChainCreateInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SwapChainCreateInfo.preTransform     = preTransform;
    SwapChainCreateInfo.imageArrayLayers = 1;
    SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    SwapChainCreateInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    SwapChainCreateInfo.clipped          = true;
    SwapChainCreateInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    
    VkResult res = vkCreateSwapchainKHR(m_core.GetDevice(), &SwapChainCreateInfo, NULL, &m_swapChainKHR);
    CHECK_VULKAN_ERROR("vkCreateSwapchainKHR error %d\n", res);    

    printf("Swap chain created\n");
    
    uint NumSwapChainImages = 0;
    res = vkGetSwapchainImagesKHR(m_core.GetDevice(), m_swapChainKHR, &NumSwapChainImages, NULL);
    CHECK_VULKAN_ERROR("vkGetSwapchainImagesKHR error %d\n", res);
    
    printf("Number of images %d\n", NumSwapChainImages);

    m_images.resize(NumSwapChainImages);
    m_presentQCmdBuffs.resize(NumSwapChainImages);
    
    res = vkGetSwapchainImagesKHR(m_core.GetDevice(), m_swapChainKHR, &NumSwapChainImages, &(m_images[0]));
    CHECK_VULKAN_ERROR("vkGetSwapchainImagesKHR error %d\n", res);
}


void OgldevVulkanApp::CreateCommandBuffer()
{
    VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolCreateInfo.queueFamilyIndex = m_core.GetQueueFamily();
    
    VkResult res = vkCreateCommandPool(m_core.GetDevice(), &cmdPoolCreateInfo, NULL, &m_presentQCmdPool);    
    CHECK_VULKAN_ERROR("vkCreateCommandPool error %d\n", res);
    
    printf("Command buffer pool created\n");
    
    VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
    cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocInfo.commandPool = m_presentQCmdPool;
    cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufAllocInfo.commandBufferCount = m_images.size();
    
    res = vkAllocateCommandBuffers(m_core.GetDevice(), &cmdBufAllocInfo, &m_presentQCmdBuffs[0]);            
    CHECK_VULKAN_ERROR("vkAllocateCommandBuffers error %d\n", res);
    
    printf("Created command buffers\n");
}


void OgldevVulkanApp::Draw()
{
    uint ImageIndex = 0;
    
    VkResult res = vkAcquireNextImageKHR(m_core.GetDevice(), m_swapChainKHR, UINT64_MAX, NULL, NULL, &ImageIndex);
    
    switch (res) {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            assert(0);
        default:
            assert(0);
    }
    
    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask    = &stageFlags;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &m_presentQCmdBuffs[ImageIndex];
    
    res = vkQueueSubmit(m_queue, 1, &submitInfo, NULL);
    
    CHECK_VULKAN_ERROR("vkQueueSubmit error %d\n", res);
    
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &m_swapChainKHR;
    presentInfo.pImageIndices      = &ImageIndex;
    
    res = vkQueuePresentKHR(m_queue, &presentInfo);
    
    CHECK_VULKAN_ERROR("vkQueuePresentKHR error %d\n" , res);
}


void OgldevVulkanApp::RecordCommandBuffers() 
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    
    VkClearColorValue clearColor = { 1.0f, 1.0f, 0.0f, 0.0f };
    VkClearValue clearValue = {};
    clearValue.color = clearColor;
    
    VkImageSubresourceRange imageRange = {};
    imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageRange.levelCount = 1;
    imageRange.layerCount = 1;
       
    VkResult res;
    
    for (uint i = 0 ; i < m_presentQCmdBuffs.size() ; i++) {             
        res = vkBeginCommandBuffer(m_presentQCmdBuffs[i], &beginInfo);
        CHECK_VULKAN_ERROR("vkBeginCommandBuffer error %d\n", res);
                
      //  vkCmdPipelineBarrier(m_presentQCmdBuffs[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
      //                       0, 0, NULL, 0, NULL, 1, &memBarrier_PresentToClear);

                     
        vkCmdClearColorImage(m_presentQCmdBuffs[i], m_images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageRange);                
        
        
      //  vkCmdPipelineBarrier(m_presentQCmdBuffs[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      //                       0, 0, NULL, 0, NULL, 1, &memBarrier_ClearToPresent);

        res = vkEndCommandBuffer(m_presentQCmdBuffs[i]);
        CHECK_VULKAN_ERROR("vkEndCommandBuffer error %d\n", res);
    }
    
    printf("Command buffers recorded\n");    
}


void OgldevVulkanApp::Init()
{
#ifdef WIN32
    
#else            
    m_pWindowControl = new XCBControl();
#endif    
    m_pWindowControl->Init(WINDOW_WIDTH, WINDOW_HEIGHT);

    m_core.Init(m_pWindowControl);
        
    vkGetDeviceQueue(m_core.GetDevice(), m_core.GetQueueFamily(), 0, &m_queue);

    EnumPhysDeviceExtProps();
    CreateSwapChain();
    CreateCommandBuffer();
    RecordCommandBuffers();
}


void OgldevVulkanApp::Run()
{    
    m_pWindowControl->PreRun();

    while (true) {
        m_pWindowControl->PollEvent();
        
        Draw();

        // Wait for work to finish before updating MVP.
        vkDeviceWaitIdle(m_core.GetDevice());
    }
}


class Tutorial51 : public OgldevVulkanApp
{
public:
    Tutorial51(const char* pAppName);
    
    ~Tutorial51();
};


Tutorial51::Tutorial51(const char* pAppName) : OgldevVulkanApp(pAppName)
{
    
}

Tutorial51::~Tutorial51()
{
    
}

int main(int argc, char** argv)
{
    Tutorial51 app("Tutorial 51");
    
    app.Init();
    
    app.Run();
    
    return 0;
}