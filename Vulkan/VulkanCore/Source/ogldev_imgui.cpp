/*

		Copyright 2024 Etay Meiri

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

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
//#include "imGuIZMOquat.h"

#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_imgui.h"
#include "ogldev_vulkan_wrapper.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080


static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

namespace OgldevVK {

ImGUIRenderer::ImGUIRenderer()
{
}

void ImGUIRenderer::Init(VulkanCore* pvkCore)
{
	VulkanRenderer::Init(pvkCore);

	bool DepthEnabled = true;

	CreateDescriptorPool();

	InitImGUI();
}


ImGUIRenderer::~ImGUIRenderer()
{
	m_pvkCore->FreeCommandBuffers(1, &m_cmdBuf);

	vkDestroyDescriptorPool(m_device, m_descriptorPool, NULL);

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


void ImGUIRenderer::CreateDescriptorPool()
{
	VkDescriptorPoolSize PoolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo PoolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = 1000 * IM_ARRAYSIZE(PoolSizes),
		.poolSizeCount = (uint32_t)IM_ARRAYSIZE(PoolSizes),
		.pPoolSizes = PoolSizes
	};

	VkResult res = vkCreateDescriptorPool(m_device, &PoolCreateInfo, NULL, &m_descriptorPool);
	CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
}


void ImGUIRenderer::InitImGUI()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	//io.Fonts->AddFontDefault();
	//io.Fonts->Build();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	 // Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(m_pvkCore->GetWindow(), true);

	VkFormat ColorFormat = m_pvkCore->GetSwapChainFormat();

	ImGui_ImplVulkan_InitInfo InitInfo = {
		.ApiVersion = VK_API_VERSION_1_3,
		.Instance = m_pvkCore->GetInstance(),
		.PhysicalDevice = m_pvkCore->GetPhysicalDevice().m_physDevice,
		.Device = m_pvkCore->GetDevice(),
		.QueueFamily = m_pvkCore->GetQueueFamily(),
		.Queue = m_pvkCore->GetQueue()->GetHandle(),
		.DescriptorPool = m_descriptorPool,
		.RenderPass = NULL,
		.MinImageCount = m_pvkCore->GetPhysicalDevice().m_surfaceCaps.minImageCount,
		.ImageCount = (u32)m_pvkCore->GetNumImages(),
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.PipelineCache = NULL,
		.Subpass = 0,
		.UseDynamicRendering = true,		
		.PipelineRenderingCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
			.pNext = NULL,
			.viewMask = 0,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &ColorFormat,
			.depthAttachmentFormat = m_pvkCore->GetDepthFormat(),
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		 },
		.Allocator = NULL,
		.CheckVkResultFn = check_vk_result
	};

	ImGui_ImplVulkan_Init(&InitInfo);

	m_pvkCore->CreateCommandBuffers(1, &m_cmdBuf);
	//InitImGUIFontsTexture();
}


void ImGUIRenderer::InitImGUIFontsTexture()
{	
	m_pvkCore->CreateCommandBuffers(1, &m_cmdBuf);

	BeginCommandBuffer(m_cmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

	//ImGui_ImplVulkan_CreateFontsTexture(m_cmdBuf);

	VkResult res = vkEndCommandBuffer(m_cmdBuf);
	CHECK_VK_RESULT(res, "vkEndCommandBuffer");

	m_pvkCore->GetQueue()->SubmitSync(m_cmdBuf);
	m_pvkCore->GetQueue()->WaitIdle();

	//ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGUIRenderer::FillCommandBuffer(VkCommandBuffer CmdBuf, int Image)
{
}


void ImGUIRenderer::OnFrame(int Image)
{
	static float f = 0.0f;
	static int counter = 0;

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = (float)m_framebufferWidth;      
	io.DisplaySize.y = (float)m_framebufferHeight;             

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();

	ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_AlwaysAutoResize);   // Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	//	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();

	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();

	VkResult result = vkResetCommandBuffer(m_cmdBuf, 0);
	CHECK_VK_RESULT(result, "vkResetCommandBuffer");

	BeginCommandBuffer(m_cmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	OgldevVK::ImageMemBarrier(m_cmdBuf, m_pvkCore->GetImage(Image), m_pvkCore->GetSwapChainFormat(),
							  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	BeginRendering(m_cmdBuf, Image);

	ImGui_ImplVulkan_RenderDrawData(draw_data, m_cmdBuf);
	
	vkCmdEndRendering(m_cmdBuf);

	OgldevVK::ImageMemBarrier(m_cmdBuf, m_pvkCore->GetImage(Image), m_pvkCore->GetSwapChainFormat(),
							  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	vkEndCommandBuffer(m_cmdBuf);

	m_pvkCore->GetQueue()->SubmitAsync(m_cmdBuf);
	//m_pvkCore->GetQueue()->WaitIdle();
}


void ImGUIRenderer::BeginRendering(VkCommandBuffer CmdBuf, int ImageIndex)
{
	VkClearValue ClearColor = {
		.color = {1.0f, 0.0f, 0.0f, 1.0f},
	};

	VkClearValue DepthValue = {
		.depthStencil = {.depth = 1.0f, .stencil = 0 }
	};

	VkRenderingAttachmentInfoKHR ColorAttachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
		.pNext = NULL,
		.imageView = m_pvkCore->GetImageView(ImageIndex),
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.resolveMode = VK_RESOLVE_MODE_NONE,
		.resolveImageView = VK_NULL_HANDLE,
		.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = ClearColor
	};

	VkRenderingAttachmentInfo DepthAttachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.pNext = NULL,
		.imageView = m_pvkCore->GetDepthView(ImageIndex),
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.resolveMode = VK_RESOLVE_MODE_NONE,
		.resolveImageView = VK_NULL_HANDLE,
		.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = DepthValue,
	};

	VkRenderingInfoKHR RenderingInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
		.renderArea = { {0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT} },
		.layerCount = 1,
		.viewMask = 0,
		.colorAttachmentCount = 1,
		.pColorAttachments = &ColorAttachment,
		.pDepthAttachment = &DepthAttachment
	};

	vkCmdBeginRendering(CmdBuf, &RenderingInfo);
}


}
