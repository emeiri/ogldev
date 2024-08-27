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

#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_finish.h"

#pragma once

namespace OgldevVK {

FinishRenderer::FinishRenderer(VulkanCore& vkCore) : VulkanRenderer(vkCore)											   
{
	bool DepthEnabled = m_vkCore.GetDepthTexture().m_image != VK_NULL_HANDLE;

	m_renderPass = m_vkCore.CreateSimpleRenderPass(DepthEnabled, false, false, RenderPassTypeFirst);

	m_frameBuffers = m_vkCore.CreateFramebuffers(m_renderPass);
}


void FinishRenderer::FillCommandBuffer(VkCommandBuffer CmdBuf, int Image)
{
	VkRect2D RenderArea = {
		.offset = { 0, 0 },
		.extent = {
			.width = m_framebufferWidth, 
			.height = m_framebufferHeight 
		}
	};

	VkRenderPassBeginInfo RenderPassBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_renderPass,
		.framebuffer = m_frameBuffers[Image],
		.renderArea = RenderArea,
		.clearValueCount = 0,
		.pClearValues = NULL
	};

	vkCmdBeginRenderPass(CmdBuf, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdEndRenderPass(CmdBuf);
}


}
