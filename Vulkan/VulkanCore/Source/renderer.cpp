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

#include "ogldev_vulkan_renderer.h"

namespace OgldevVK {

VulkanRenderer::VulkanRenderer(VulkanCore& vkCore) : m_vkCore(vkCore)
{
	m_device = vkCore.GetDevice();

	vkCore.GetFramebufferSize(m_framebufferWidth, m_framebufferHeight);
}

VulkanRenderer::~VulkanRenderer()
{
	m_vkCore.DestroyFramebuffers(m_frameBuffers);

	vkDestroyRenderPass(m_device, m_renderPass, NULL);

	for (int i = 0; i < m_uniformBuffers.size(); i++) {
		m_uniformBuffers[i].Destroy(m_device);
	}

	delete m_pPipeline;
}


void VulkanRenderer::BeginRenderPass(VkCommandBuffer CmdBuf, int Image)
{
	VkRect2D RenderArea = {
		.offset = { 0, 0 },
		.extent = {
			.width = (u32)m_framebufferWidth,
			.height = (u32)m_framebufferHeight
		}
	};

	VkRenderPassBeginInfo RenderPassBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = nullptr,
		.renderPass = m_renderPass,
		.framebuffer = m_frameBuffers[Image],
		.renderArea = RenderArea
	};

	vkCmdBeginRenderPass(CmdBuf, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_pPipeline->Bind(CmdBuf, Image);
}


}