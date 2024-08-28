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

	//for (int i = 0; i < m_VBs.size() ; i++) {
		m_VBs[0].Destroy(m_device);
	//}
}


void CanvasRenderer::CreateShaders()
{
	m_vs = CreateShaderModuleFromText(m_device, "../VulkanCore/Shaders/lines.vert");

	m_fs = CreateShaderModuleFromText(m_device, "../VulkanCore/Shaders/lines.frag");
}


void CanvasRenderer::CreateVertexBuffers()
{
	int NumImages = 1;// m_vkCore.GetNumImages();

	m_VBs.resize(NumImages);

	for (int i = 0; i < NumImages; i++) {
		m_VBs[i] = m_vkCore.CreateVertexBuffer(NULL, MaxLinesDataSize, true);
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


void CanvasRenderer::UpdateUniformBuffer(int Image, const glm::mat4& WVP, float Time)
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

		m_vkCore.UploadBufferData(m_VBs[0].m_mem, 0, m_lines.data(), BufferSize);
	}
}


void CanvasRenderer::Clear()
{
	m_lines.clear();
}

void CanvasRenderer::Line(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& c)
{
	m_lines.push_back({ .Position = p1, .Color = c });
	m_lines.push_back({ .Position = p2, .Color = c });
}

void CanvasRenderer::Plane3D(const glm::vec3& o, const glm::vec3& v1, const glm::vec3& v2, int n1, int n2, 
							 float s1, float s2, const glm::vec4& color, const glm::vec4& outlineColor)
{
	Line(o - s1 / 2.0f * v1 - s2 / 2.0f * v2, o - s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);
	Line(o + s1 / 2.0f * v1 - s2 / 2.0f * v2, o + s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);

	Line(o - s1 / 2.0f * v1 + s2 / 2.0f * v2, o + s1 / 2.0f * v1 + s2 / 2.0f * v2, outlineColor);
	Line(o - s1 / 2.0f * v1 - s2 / 2.0f * v2, o + s1 / 2.0f * v1 - s2 / 2.0f * v2, outlineColor);

	for (int i = 1; i < n1; i++) {
		float t = ((float)i - (float)n1 / 2.0f) * s1 / (float)n1;
		const glm::vec3 o1 = o + t * v1;
		Line(o1 - s2 / 2.0f * v2, o1 + s2 / 2.0f * v2, color);
	}

	for (int i = 1; i < n2; i++)
	{
		const float t = ((float)i - (float)n2 / 2.0f) * s2 / (float)n2;
		const glm::vec3 o2 = o + t * v2;
		Line(o2 - s1 / 2.0f * v1, o2 + s1 / 2.0f * v1, color);
	}
}



}
