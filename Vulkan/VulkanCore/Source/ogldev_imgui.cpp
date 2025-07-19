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

#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_imgui.h"
#include "ogldev_vulkan_wrapper.h"

static void CheckVKResult(VkResult err)
{
	if (err == 0) {
		return;
	}
		
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);

	if (err < 0) {
		abort();
	}
}


bool IsMouseControlledByImGUI()
{
	ImGuiIO& io = ImGui::GetIO();

	bool ret = io.WantCaptureMouse;
		
	return ret;
}


namespace OgldevVK {

void ImGUIRenderer::Init(VulkanCore* pvkCore)
{
	m_pvkCore = pvkCore;

	m_pvkCore->GetFramebufferSize(m_framebufferWidth, m_framebufferHeight);

	CreateDescriptorPool();

	InitImGUI();
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

	VkResult res = vkCreateDescriptorPool(m_pvkCore->GetDevice(), &PoolCreateInfo, NULL, &m_descriptorPool);
	CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
}


void ImGUIRenderer::InitImGUI()
{
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos; 
	io.DisplaySize.x = (float)m_framebufferWidth;
	io.DisplaySize.y = (float)m_framebufferHeight;

	ImGui::GetStyle().FontScaleMain = 1.5f;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	bool InstallGLFWCallbacks = true;
	ImGui_ImplGlfw_InitForVulkan(m_pvkCore->GetWindow(), InstallGLFWCallbacks);

	VkFormat ColorFormat = m_pvkCore->GetSwapChainFormat();

	ImGui_ImplVulkan_InitInfo InitInfo = {
		.ApiVersion = m_pvkCore->GetInstanceVersion(),
		.Instance = m_pvkCore->GetInstance(),
		.PhysicalDevice = m_pvkCore->GetPhysicalDevice().m_physDevice,
		.Device = m_pvkCore->GetDevice(),
		.QueueFamily = m_pvkCore->GetQueueFamily(),
		.Queue = m_pvkCore->GetQueue()->GetHandle(),
		.DescriptorPool = m_descriptorPool,
		.RenderPass = NULL,		// assume dynamic rendering
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
		.CheckVkResultFn = CheckVKResult
	};

	ImGui_ImplVulkan_Init(&InitInfo);

	m_cmdBufs.resize(m_pvkCore->GetNumImages());
	m_pvkCore->CreateCommandBuffers(m_pvkCore->GetNumImages(), m_cmdBufs.data());
}


void ImGUIRenderer::Destroy()
{
	m_pvkCore->FreeCommandBuffers((u32)m_cmdBufs.size(), m_cmdBufs.data());

	vkDestroyDescriptorPool(m_pvkCore->GetDevice(), m_descriptorPool, NULL);

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


// Must be called after the ImGUI frame was prepared on the application side!
VkCommandBuffer ImGUIRenderer::PrepareCommandBuffer(int Image)
{
	BeginCommandBuffer(m_cmdBufs[Image], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	m_pvkCore->BeginDynamicRendering(m_cmdBufs[Image], Image, NULL, NULL);

	ImDrawData* pDrawData = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(pDrawData, m_cmdBufs[Image]);

	vkCmdEndRendering(m_cmdBufs[Image]);

	OgldevVK::ImageMemBarrier(m_cmdBufs[Image], m_pvkCore->GetImage(Image), m_pvkCore->GetSwapChainFormat(),
		                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	vkEndCommandBuffer(m_cmdBufs[Image]);

	return m_cmdBufs[Image];
}


}
