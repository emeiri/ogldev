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
#include "ogldev_vulkan_model.h"
#include "ogldev_vulkan_shader.h"

#pragma once

namespace OgldevVK {

ModelRenderer::ModelRenderer(VulkanCore& vkCore, const char* pModelFilename, const char* pTextureFilename, size_t UniformDataSize) : VulkanRenderer(vkCore)
{
	bool DepthEnabled = m_vkCore.GetDepthTexture().m_image != VK_NULL_HANDLE;

	m_renderPass = m_vkCore.CreateSimpleRenderPass(DepthEnabled, false, false, RenderPassTypeDefault);

	m_frameBuffers = m_vkCore.CreateFramebuffers(m_renderPass);

	SimpleMesh Mesh = vkCore.LoadSimpleMesh(pModelFilename);

	m_vb = Mesh.m_vb;
	m_ib = Mesh.m_ib;
	m_numVertices = Mesh.m_numVertices;
	m_numIndices = Mesh.m_numIndices;
	m_vertexBufferSize = Mesh.m_vertexBufferSize;
	m_indexBufferSize = Mesh.m_indexBufferSize;

	vkCore.CreateTexture(pTextureFilename, m_texture);

	m_uniformBuffers = vkCore.CreateUniformBuffers(UniformDataSize);

	CreateShaders();

	int NumImages = vkCore.GetNumImages();

	m_pPipeline = new OgldevVK::GraphicsPipeline(m_device, m_framebufferWidth, m_framebufferHeight, m_renderPass, 
												 m_vs, m_fs, m_vb.m_buffer, m_vertexBufferSize, m_ib.m_buffer, m_indexBufferSize,
												 NumImages, m_uniformBuffers, (int)UniformDataSize, &m_texture);
}

ModelRenderer::~ModelRenderer()
{
	vkDestroyShaderModule(m_device, m_vs, NULL);
	vkDestroyShaderModule(m_device, m_fs, NULL);
	m_vkCore.DestroyTexture(m_texture);
	m_vb.Destroy(m_device);
	m_ib.Destroy(m_device);
}


void ModelRenderer::CreateShaders()
{
	m_vs = CreateShaderModuleFromText(m_device, "test.vert");

	m_fs = CreateShaderModuleFromText(m_device, "test.frag");
}


void ModelRenderer::FillCommandBuffer(VkCommandBuffer CmdBuf, int Image)
{
	BeginRenderPass(CmdBuf, Image);

	vkCmdDraw(CmdBuf, m_numIndices, 1, 0, 0);

	vkCmdEndRenderPass(CmdBuf);
}


void ModelRenderer::UpdateUniformBuffers(int Image, const void* pData, size_t Size)
{
	m_uniformBuffers[Image].Update(m_device, pData, Size);
}

}
