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

#pragma once

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

namespace OgldevVK {

VkDescriptorPool g_DescriptorPool = NULL;

ImGUIRenderer::ImGUIRenderer(VulkanCore& vkCore) : VulkanRenderer(vkCore)
{
	bool DepthEnabled = m_vkCore.GetDepthTexture().m_image != VK_NULL_HANDLE;

	m_renderPass = m_vkCore.CreateSimpleRenderPass(DepthEnabled, true, false, (OgldevVK::RenderPassType)(RenderPassTypeFirst | RenderPassTypeLast));

	m_frameBuffers = m_vkCore.CreateFramebuffers(m_renderPass);

	//m_pPipeline = new OgldevVK::GraphicsPipeline(m_device, m_framebufferWidth, m_framebufferHeight, m_renderPass, 
	//											 NULL, NULL, NULL, 0, NULL, 0,
	//											 m_vkCore.GetNumImages(), m_uniformBuffers, 0, NULL, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

	// Create Descriptor Pool
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
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
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		VkResult res = vkCreateDescriptorPool(m_device, &pool_info, NULL, &g_DescriptorPool);
		CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
	}

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

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_vkCore.GetInstance();
	init_info.PhysicalDevice = m_vkCore.GetPhysicalDevice().m_physDevice;
	init_info.Device = m_vkCore.GetDevice();;
	init_info.QueueFamily = m_vkCore.GetQueueFamily();
	init_info.Queue = m_vkCore.GetQueue()->GetHandle();
	init_info.PipelineCache = NULL;
	init_info.DescriptorPool = g_DescriptorPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = m_vkCore.GetPhysicalDevice().m_surfaceCaps.minImageCount;
	init_info.ImageCount = m_vkCore.GetNumImages();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = NULL;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, m_renderPass);

	VkCommandBuffer command_buffer = NULL;
	m_vkCore.CreateCommandBuffers(1, &command_buffer);
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VkResult res = vkBeginCommandBuffer(command_buffer, &begin_info);
	CHECK_VK_RESULT(res, "vkBeginCommandBuffer");

	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &command_buffer;
	res = vkEndCommandBuffer(command_buffer);
	CHECK_VK_RESULT(res, "vkEndCommandBuffer");
	m_vkCore.GetQueue()->SubmitSync(command_buffer);
	m_vkCore.GetQueue()->WaitIdle();
	ImGui_ImplVulkan_DestroyFontUploadObjects();

}


ImGUIRenderer::~ImGUIRenderer()
{
//	vkDestroyShaderModule(m_device, m_vs, NULL);
	//vkDestroyShaderModule(m_device, m_fs, NULL);


}


/*void ImGUIRenderer::CreateShaders()
{
	m_vs = CreateShaderModuleFromText(m_device, "../VulkanCore/Shaders/lines.vert");

	m_fs = CreateShaderModuleFromText(m_device, "../VulkanCore/Shaders/lines.frag");
}*/




void ImGUIRenderer::FillCommandBuffer(VkCommandBuffer CmdBuf, int Image)
{
	static float f = 0.0f;
	static int counter = 0;

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = (float)m_framebufferWidth;      
	io.DisplaySize.y = (float)m_framebufferHeight;             

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();

	ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_AlwaysAutoResize);                          // Create a window called "Hello, world!" and append into it.

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

	BeginRenderPass(CmdBuf, Image);

	ImGui_ImplVulkan_RenderDrawData(draw_data, CmdBuf);

	vkCmdEndRenderPass(CmdBuf);
}







}
