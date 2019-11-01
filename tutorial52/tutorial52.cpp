/*

	Copyright 2017 Etay Meiri

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

    Tutorial 52 - Vulkan First Triangle
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

    void CreateSwapChain();
    void CreateCommandBuffer();
    void CreateRenderPass();
    void CreateFramebuffer();
    void CreateShaders();
    void CreatePipeline();
    void RecordCommandBuffers();
    void RenderScene();

    std::string m_appName;
    VulkanWindowControl* m_pWindowControl;
    OgldevVulkanCore m_core;    
    std::vector<VkImage> m_images;
    VkSwapchainKHR m_swapChainKHR;
    VkQueue m_queue;
    std::vector<VkCommandBuffer> m_cmdBufs;
    VkCommandPool m_cmdBufPool;
    std::vector<VkImageView> m_views;	
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_fbs;
    VkShaderModule m_vsModule;
    VkShaderModule m_fsModule;
    VkPipeline m_pipeline;
};


OgldevVulkanApp::OgldevVulkanApp(const char* pAppName) : m_core(pAppName)
{
    m_appName = std::string(pAppName);
}


OgldevVulkanApp::~OgldevVulkanApp()
{   
}
    

void OgldevVulkanApp::CreateSwapChain()
{          
    const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_core.GetSurfaceCaps();
         
    assert(SurfaceCaps.currentExtent.width != -1);
                   
    uint NumImages = 2;

    assert(NumImages >= SurfaceCaps.minImageCount);
    assert(NumImages <= SurfaceCaps.maxImageCount);
      
    VkSwapchainCreateInfoKHR SwapChainCreateInfo = {};
    
    SwapChainCreateInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapChainCreateInfo.surface          = m_core.GetSurface();
    SwapChainCreateInfo.minImageCount    = NumImages;
    SwapChainCreateInfo.imageFormat      = m_core.GetSurfaceFormat().format;
    SwapChainCreateInfo.imageColorSpace  = m_core.GetSurfaceFormat().colorSpace;
    SwapChainCreateInfo.imageExtent      = SurfaceCaps.currentExtent;
    SwapChainCreateInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SwapChainCreateInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
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
    assert(NumImages == NumSwapChainImages);
    
    printf("Number of images %d\n", NumSwapChainImages);

    m_images.resize(NumSwapChainImages);
    m_views.resize(NumSwapChainImages);
    m_cmdBufs.resize(NumSwapChainImages);
    
    res = vkGetSwapchainImagesKHR(m_core.GetDevice(), m_swapChainKHR, &NumSwapChainImages, &(m_images[0]));
    CHECK_VULKAN_ERROR("vkGetSwapchainImagesKHR error %d\n", res);
}


void OgldevVulkanApp::CreateCommandBuffer()
{
    VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolCreateInfo.queueFamilyIndex = m_core.GetQueueFamily();
    
    VkResult res = vkCreateCommandPool(m_core.GetDevice(), &cmdPoolCreateInfo, NULL, &m_cmdBufPool);    
    CHECK_VULKAN_ERROR("vkCreateCommandPool error %d\n", res);
    
    printf("Command buffer pool created\n");
    
    VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
    cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocInfo.commandPool = m_cmdBufPool;
    cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufAllocInfo.commandBufferCount = m_images.size();
    
    res = vkAllocateCommandBuffers(m_core.GetDevice(), &cmdBufAllocInfo, &m_cmdBufs[0]);            
    CHECK_VULKAN_ERROR("vkAllocateCommandBuffers error %d\n", res);
    
    printf("Created command buffers\n");
}


void OgldevVulkanApp::RecordCommandBuffers() 
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    
    VkClearColorValue clearColor = { 164.0f/256.0f, 30.0f/256.0f, 34.0f/256.0f, 0.0f };
    VkClearValue clearValue = {};
    clearValue.color = clearColor;
    
    VkImageSubresourceRange imageRange = {};
    imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageRange.levelCount = 1;
    imageRange.layerCount = 1;
    
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;   
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent.width = WINDOW_WIDTH;
    renderPassInfo.renderArea.extent.height = WINDOW_HEIGHT;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    VkViewport viewport = { 0 };
    viewport.height = (float)WINDOW_HEIGHT;
    viewport.width = (float)WINDOW_WIDTH;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
        
    VkRect2D scissor = { 0 };
    scissor.extent.width = WINDOW_WIDTH;
    scissor.extent.height = WINDOW_HEIGHT;
    scissor.offset.x = 0;
    scissor.offset.y = 0;

    for (uint i = 0 ; i < m_cmdBufs.size() ; i++) {            
        VkResult res = vkBeginCommandBuffer(m_cmdBufs[i], &beginInfo);
        CHECK_VULKAN_ERROR("vkBeginCommandBuffer error %d\n", res);
        renderPassInfo.framebuffer = m_fbs[i];

        vkCmdBeginRenderPass(m_cmdBufs[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_cmdBufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
        
        vkCmdSetViewport(m_cmdBufs[i], 0, 1, &viewport);

        vkCmdSetScissor(m_cmdBufs[i], 0, 1, &scissor);

        vkCmdDraw(m_cmdBufs[i], 3, 1, 0, 0);
        
        vkCmdEndRenderPass(m_cmdBufs[i]);
               
        res = vkEndCommandBuffer(m_cmdBufs[i]);
        CHECK_VULKAN_ERROR("vkEndCommandBuffer error %d\n", res);
    }
    
    printf("Command buffers recorded\n");    
}


void OgldevVulkanApp::RenderScene()
{
    uint ImageIndex = 0;
    
    VkResult res = vkAcquireNextImageKHR(m_core.GetDevice(), m_swapChainKHR, UINT64_MAX, NULL, NULL, &ImageIndex);
    CHECK_VULKAN_ERROR("vkAcquireNextImageKHR error %d\n" , res);
   
    VkSubmitInfo submitInfo = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &m_cmdBufs[ImageIndex];
    
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



void OgldevVulkanApp::CreateRenderPass()
{
    VkAttachmentReference attachRef = {};
    attachRef.attachment = 0;
    attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDesc = {};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &attachRef;

    VkAttachmentDescription attachDesc = {};    
    attachDesc.format = m_core.GetSurfaceFormat().format;
    attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachDesc;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDesc;
        
    VkResult res = vkCreateRenderPass(m_core.GetDevice(), &renderPassCreateInfo, NULL, &m_renderPass);
    CHECK_VULKAN_ERROR("vkCreateRenderPass error %d\n", res);

    printf("Created a render pass\n");
}


void OgldevVulkanApp::CreateFramebuffer()
{
    m_fbs.resize(m_images.size());
    
    VkResult res;
            
    for (uint i = 0 ; i < m_images.size() ; i++) {
        VkImageViewCreateInfo ViewCreateInfo = {};
        ViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ViewCreateInfo.image = m_images[i];
        ViewCreateInfo.format = m_core.GetSurfaceFormat().format;
        ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        ViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ViewCreateInfo.subresourceRange.baseMipLevel = 0;
        ViewCreateInfo.subresourceRange.levelCount = 1;
        ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ViewCreateInfo.subresourceRange.layerCount = 1;    

        res = vkCreateImageView(m_core.GetDevice(), &ViewCreateInfo, NULL, &m_views[i]);
        CHECK_VULKAN_ERROR("vkCreateImageView error %d\n", res);
        
        VkFramebufferCreateInfo fbCreateInfo = {};
        fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbCreateInfo.renderPass = m_renderPass;
        fbCreateInfo.attachmentCount = 1;
        fbCreateInfo.pAttachments = &m_views[i];
        fbCreateInfo.width = WINDOW_WIDTH;
        fbCreateInfo.height = WINDOW_HEIGHT;
        fbCreateInfo.layers = 1;

        res = vkCreateFramebuffer(m_core.GetDevice(), &fbCreateInfo, NULL, &m_fbs[i]);
        CHECK_VULKAN_ERROR("vkCreateFramebuffer error %d\n", res);
    }
   
    printf("Frame buffers created\n");
}


void OgldevVulkanApp::CreateShaders()
{
    m_vsModule = VulkanCreateShaderModule(m_core.GetDevice(), "Shaders/vs.spv");
    assert(m_vsModule);

    m_fsModule = VulkanCreateShaderModule(m_core.GetDevice(), "Shaders/fs.spv");
    assert(m_fsModule);
}


#define VERTEX_BUFFER_BIND_ID 0

void OgldevVulkanApp::CreatePipeline()
{
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};
    
    shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageCreateInfo[0].module = m_vsModule;
    shaderStageCreateInfo[0].pName = "main";
    shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageCreateInfo[1].module = m_fsModule;
    shaderStageCreateInfo[1].pName = "main";   
	    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    VkPipelineInputAssemblyStateCreateInfo pipelineIACreateInfo = {};
    pipelineIACreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineIACreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    
    VkViewport vp = {};
    vp.x = 0.0f;
    vp.y = 0.0f;
    vp.width  = (float)WINDOW_WIDTH;
    vp.height = (float)WINDOW_HEIGHT;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
        
    VkPipelineViewportStateCreateInfo vpCreateInfo = {};
    vpCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpCreateInfo.viewportCount = 1;
    vpCreateInfo.pViewports = &vp;
       
    VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
    rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rastCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rastCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rastCreateInfo.lineWidth = 1.0f;
    
    VkPipelineMultisampleStateCreateInfo pipelineMSCreateInfo = {};
    pipelineMSCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    
    VkPipelineColorBlendAttachmentState blendAttachState = {};
    blendAttachState.colorWriteMask = 0xf;
    
    VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
    blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    blendCreateInfo.attachmentCount = 1;
    blendCreateInfo.pAttachments = &blendAttachState;
 
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = ARRAY_SIZE_IN_ELEMENTS(shaderStageCreateInfo);
    pipelineInfo.pStages = &shaderStageCreateInfo[0];
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &pipelineIACreateInfo;
    pipelineInfo.pViewportState = &vpCreateInfo;
    pipelineInfo.pRasterizationState = &rastCreateInfo;
    pipelineInfo.pMultisampleState = &pipelineMSCreateInfo;
    pipelineInfo.pColorBlendState = &blendCreateInfo;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.basePipelineIndex = -1;
    
    VkResult res = vkCreateGraphicsPipelines(m_core.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &m_pipeline);
    CHECK_VULKAN_ERROR("vkCreateGraphicsPipelines error %d\n", res);
    
    printf("Graphics pipeline created\n");
}


void OgldevVulkanApp::Init()
{
#ifdef WIN32
    m_pWindowControl = new Win32Control(m_appName.c_str());
#else            
    m_pWindowControl = new XCBControl();
#endif    
    m_pWindowControl->Init(WINDOW_WIDTH, WINDOW_HEIGHT);

    m_core.Init(m_pWindowControl);
        
    vkGetDeviceQueue(m_core.GetDevice(), m_core.GetQueueFamily(), 0, &m_queue);

    CreateSwapChain();
    CreateCommandBuffer();
    CreateRenderPass();
    CreateFramebuffer();
    CreateShaders();
    CreatePipeline();
    RecordCommandBuffers();    
}


void OgldevVulkanApp::Run()
{    
    while (true) {        
        RenderScene();
    }
}


int main(int argc, char** argv)
{
    OgldevVulkanApp app("Tutorial 52");
    
    app.Init();
    
    app.Run();
    
    return 0;
}

