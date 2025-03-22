/*

		Copyright 2025 Etay Meiri

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

#include <vulkan/vulkan.h>

#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_model.h"
#include "ogldev_vulkan_graphics_pipeline_v2.h"

namespace OgldevVK {


#define UNIFORM_BUFFER_SIZE sizeof(glm::mat4)


void VkModel::Destroy()
{
	m_vb.Destroy(m_pVulkanCore->GetDevice());
	m_ib.Destroy(m_pVulkanCore->GetDevice());

	for (int i = 0; i < m_uniformBuffers.size(); i++) {
		m_uniformBuffers[i].Destroy(m_pVulkanCore->GetDevice());
	}
}


Texture* VkModel::AllocTexture2D()
{
	assert(m_pVulkanCore);

	return new VulkanTexture(m_pVulkanCore);
}


void VkModel::PopulateBuffers(vector<Vertex>& Vertices)
{
	m_vb = m_pVulkanCore->CreateVertexBuffer(Vertices.data(), ARRAY_SIZE_IN_BYTES(Vertices));

	m_ib = m_pVulkanCore->CreateVertexBuffer(m_Indices.data(), ARRAY_SIZE_IN_BYTES(m_Indices));

	m_uniformBuffers = m_pVulkanCore->CreateUniformBuffers(UNIFORM_BUFFER_SIZE * m_Meshes.size());

	m_vertexSize = sizeof(Vertex);
}


void VkModel::CreateDescriptorSets(GraphicsPipelineV2& Pipeline)
{
	ModelDesc md;

	CreateModelDescriptor(md);

	int NumSubmeshes = (int)m_Meshes.size();
	Pipeline.AllocateDescriptorSets(NumSubmeshes, m_descriptorSets);

	Pipeline.UpdateDescriptorSets(md, m_descriptorSets);
}


void VkModel::CreateModelDescriptor(ModelDesc& md)
{
	md.m_vb = m_vb.m_buffer;
	md.m_ib = m_ib.m_buffer;

	md.m_uniforms.resize(m_pVulkanCore->GetNumImages());

	for (int ImageIndex = 0; ImageIndex < m_pVulkanCore->GetNumImages(); ImageIndex++) {
		md.m_uniforms[ImageIndex] = m_uniformBuffers[ImageIndex].m_buffer;
	}

	md.m_materials.resize(m_Meshes.size());
	md.m_ranges.resize(m_Meshes.size());

	int NumSubmeshes = (int)m_Meshes.size();

	for (int SubmeshIndex = 0; SubmeshIndex < NumSubmeshes; SubmeshIndex++) {
		int MaterialIndex = m_Meshes[SubmeshIndex].MaterialIndex;

		if ((MaterialIndex >= 0) && (m_Materials[MaterialIndex].pDiffuse)) {
			Texture* pDiffuse = m_Materials[MaterialIndex].pDiffuse;
			md.m_materials[SubmeshIndex].m_sampler = pDiffuse->m_sampler;
			md.m_materials[SubmeshIndex].m_imageView = pDiffuse->m_view;
		}

		size_t offset = m_Meshes[SubmeshIndex].BaseVertex * m_vertexSize;
		size_t range = m_Meshes[SubmeshIndex].NumVertices * m_vertexSize;
		md.m_ranges[SubmeshIndex].m_vbRange = { .m_offset = offset, .m_range = range };

		offset = m_Meshes[SubmeshIndex].BaseIndex * sizeof(u32);
		range = m_Meshes[SubmeshIndex].NumIndices * sizeof(u32);
		md.m_ranges[SubmeshIndex].m_ibRange = { .m_offset = offset, .m_range = range };

		offset = SubmeshIndex * UNIFORM_BUFFER_SIZE;
		range = UNIFORM_BUFFER_SIZE;
		md.m_ranges[SubmeshIndex].m_uniformRange = { .m_offset = offset, .m_range = range };
	}
}


void VkModel::RecordCommandBuffer(VkCommandBuffer CmdBuf, GraphicsPipelineV2& Pipeline, int ImageIndex)
{
	u32 InstanceCount = 1;
	u32 FirstInstance = 0;
	u32 BaseVertex = 0;

	for (u32 SubmeshIndex = 0; SubmeshIndex < m_Meshes.size(); SubmeshIndex++) {
		vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.GetPipelineLayout(),
			0,  // firstSet
			1,  // descriptorSetCount
			&m_descriptorSets[ImageIndex][SubmeshIndex],
			0,	// dynamicOffsetCount
			NULL);	// pDynamicOffsets

		vkCmdDraw(CmdBuf, m_Meshes[SubmeshIndex].NumIndices, InstanceCount, BaseVertex, FirstInstance);
	}
}


void VkModel::Update(int ImageIndex, const glm::mat4& Transformation)
{
	std::vector<glm::mat4> Transformations(m_Meshes.size());

	for (u32 SubmeshIndex = 0; SubmeshIndex < Transformations.size(); SubmeshIndex++) {
		glm::mat4 MeshTransform = glm::make_mat4(m_Meshes[SubmeshIndex].Transformation.data());
		MeshTransform = glm::transpose(MeshTransform);
		Transformations[SubmeshIndex] = Transformation * MeshTransform;
	}

	m_uniformBuffers[ImageIndex].Update(m_pVulkanCore->GetDevice(), Transformations.data(), ARRAY_SIZE_IN_BYTES(Transformations));
}

}