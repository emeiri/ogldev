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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imGuIZMOquat.h"

#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_imgui.h"
#include "ogldev_vulkan_wrapper.h"

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

namespace OgldevVK {

ImGUIRenderer::ImGUIRenderer(VulkanCore& vkCore) : VulkanRenderer(vkCore)
{
	bool DepthEnabled = true;

	m_renderPass = m_vkCore.CreateSimpleRenderPass(DepthEnabled, false, false, RenderPassTypeLast);

	m_frameBuffers = m_vkCore.CreateFramebuffers(m_renderPass);

	CreateDescriptorPool();

	InitImGUI();
}


ImGUIRenderer::~ImGUIRenderer()
{
	m_vkCore.FreeCommandBuffers(1, &m_cmdBuf);

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

	io.Fonts->AddFontDefault();
	io.Fonts->Build();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	 // Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(m_vkCore.GetWindow(), true);

	ImGui_ImplVulkan_InitInfo InitInfo = {
		.Instance = m_vkCore.GetInstance(),
		.PhysicalDevice = m_vkCore.GetPhysicalDevice().m_physDevice,
		.Device = m_vkCore.GetDevice(),
		.QueueFamily = m_vkCore.GetQueueFamily(),
		.Queue = m_vkCore.GetQueue()->GetHandle(),
		.PipelineCache = NULL,
		.DescriptorPool = m_descriptorPool,
		.Subpass = 0,
		.MinImageCount = m_vkCore.GetPhysicalDevice().m_surfaceCaps.minImageCount,
		.ImageCount = (u32)m_vkCore.GetNumImages(),
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.Allocator = NULL,
		.CheckVkResultFn = check_vk_result
	};

	ImGui_ImplVulkan_Init(&InitInfo, m_renderPass);

	InitImGUIFontsTexture();
}


void ImGUIRenderer::InitImGUIFontsTexture()
{	
	m_vkCore.CreateCommandBuffers(1, &m_cmdBuf);

	BeginCommandBuffer(m_cmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

	ImGui_ImplVulkan_CreateFontsTexture(m_cmdBuf);

	VkResult res = vkEndCommandBuffer(m_cmdBuf);
	CHECK_VK_RESULT(res, "vkEndCommandBuffer");

	m_vkCore.GetQueue()->SubmitSync(m_cmdBuf);
	m_vkCore.GetQueue()->WaitIdle();

	ImGui_ImplVulkan_DestroyFontUploadObjects();
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

	BeginCommandBuffer(m_cmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

	BeginRenderPass(m_cmdBuf, Image);

	ImGui_ImplVulkan_RenderDrawData(draw_data, m_cmdBuf);

	vkCmdEndRenderPass(m_cmdBuf);

	vkEndCommandBuffer(m_cmdBuf);

	m_vkCore.GetQueue()->SubmitSync(m_cmdBuf);
	m_vkCore.GetQueue()->WaitIdle();
}

}
