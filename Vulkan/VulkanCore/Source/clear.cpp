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
#include "ogldev_vulkan_clear.h"

#pragma once

namespace OgldevVK {

ClearRenderer::ClearRenderer(VulkanCore& vkCore) : VulkanRenderer(vkCore)
												   
{
	m_clearDepth = m_vkCore.GetDepthTexture().m_image != VK_NULL_HANDLE;

	m_renderPass = m_vkCore.CreateSimpleRenderPass(m_clearDepth, true, true, RenderPassTypeFirst);

	m_frameBuffers = m_vkCore.CreateFramebuffers(m_renderPass);
//	createColorAndDepthFramebuffers(vkDev, renderPass_, depthTexture.imageView, swapchainFramebuffers_);
}


void ClearRenderer::FillCommandBuffer(VkCommandBuffer CmdBuf, int Image)
{
	VkClearValue ClearValues[2] = {
		VkClearValue {.
			color = { 1.0f, 1.0f, 1.0f, 1.0f } 
		},
		VkClearValue {
			.depthStencil = { 1.0f, 0 } 
		}
	};

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
		.clearValueCount = (u32)(m_clearDepth ? 2 : 1),
		.pClearValues = &ClearValues[0]
	};

	vkCmdBeginRenderPass(CmdBuf, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdEndRenderPass(CmdBuf);
}


}
