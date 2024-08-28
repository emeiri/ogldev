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
#include "ogldev_vulkan_canvas.h"
#include "ogldev_vulkan_shader.h"

#pragma once

namespace OgldevVK {

CanvasRenderer::CanvasRenderer(VulkanCore& vkCore) : VulkanRenderer(vkCore)
{
	bool DepthEnabled = m_vkCore.GetDepthTexture().m_image != VK_NULL_HANDLE;

	m_renderPass = m_vkCore.CreateSimpleRenderPass(DepthEnabled, false, false, RenderPassTypeDefault);

	m_frameBuffers = m_vkCore.CreateFramebuffers(m_renderPass);

	CreateVertexBuffers();

	size_t UniformDataSize = sizeof(UniformBuffer);
	m_uniformBuffers = vkCore.CreateUniformBuffers(UniformDataSize);

	CreateShaders();

	int NumImages = vkCore.GetNumImages();

	m_pPipeline = new OgldevVK::GraphicsPipeline(m_device, m_framebufferWidth, m_framebufferHeight, m_renderPass, 
												 m_vs, m_fs, m_VBs[0].m_buffer, MaxLinesDataSize, NULL, 0,
												 NumImages, m_uniformBuffers, (int)UniformDataSize, NULL, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
}

CanvasRenderer::~CanvasRenderer()
{
	vkDestroyShaderModule(m_device, m_vs, NULL);
	vkDestroyShaderModule(m_device, m_fs, NULL);

	for (int i = 0; i < m_VBs.size() ; i++) {
		m_VBs[i].Destroy(m_device);
	}
}


void CanvasRenderer::CreateShaders()
{
	m_vs = CreateShaderModuleFromText(m_device, "test.vert");

	m_fs = CreateShaderModuleFromText(m_device, "test.frag");
}


void CanvasRenderer::CreateVertexBuffers()
{
	int NumImages = 1;// m_vkCore.GetNumImages();

	m_VBs.resize(NumImages);

	for (int i = 0; i < NumImages; i++) {
		m_VBs[i] = m_vkCore.CreateVertexBuffer(NULL, MaxLinesDataSize);
	}
}


void CanvasRenderer::FillCommandBuffer(VkCommandBuffer CmdBuf, int Image)
{
	if (m_lines.size() > 0) {
		BeginRenderPass(CmdBuf, Image);

		vkCmdDraw(CmdBuf, (u32)m_lines.size(), 1, 0, 0);

		vkCmdEndRenderPass(CmdBuf);
	}
}


void CanvasRenderer::UpdateUniformBuffer(const glm::mat4& WVP, float Time, int Image)
{
	UniformBuffer UBO = {
		.WVP = WVP,
		.Time = Time
	};

	m_uniformBuffers[Image].Update(m_device, &UBO, sizeof(UBO));
}


void CanvasRenderer::UpdateBuffer(int Image)
{
	if (m_lines.size() > 0) {
		VkDeviceSize BufferSize = m_lines.size() * sizeof(VertexData);

		m_vkCore.UploadBufferData(m_VBs[Image].m_mem, 0, m_lines.data(), BufferSize);
	}
}


}
