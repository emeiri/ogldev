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
#include "ogldev_vulkan_graphics_pipeline.h"

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
	//	printf("%d\n", sizeof(Vertices[0]));
	m_ib = m_pVulkanCore->CreateVertexBuffer(m_Indices.data(), ARRAY_SIZE_IN_BYTES(m_Indices));

	m_uniformBuffers = m_pVulkanCore->CreateUniformBuffers(UNIFORM_BUFFER_SIZE * m_Meshes.size());
}


void VkModel::CreateDescriptorSets(GraphicsPipeline* pPipeline)
{
	ModelDesc ModelDesc;

	ModelDesc.m_vb = m_vb.m_buffer;
	ModelDesc.m_ib = m_ib.m_buffer;

	int MaterialIndex = m_Meshes[0].MaterialIndex;

	if ((MaterialIndex >= 0) && (m_Materials[MaterialIndex].pDiffuse)) {
		Texture* pDiffuse = m_Materials[MaterialIndex].pDiffuse;
		ModelDesc.m_sampler = pDiffuse->m_sampler;
		ModelDesc.m_imageView = pDiffuse->m_view;
	}

	ModelDesc.m_uniforms.resize(m_pVulkanCore->GetNumImages());

	for (int i = 0; i < m_pVulkanCore->GetNumImages(); i++) {
		ModelDesc.m_uniforms[i] = m_uniformBuffers[i].m_buffer;
	}

	ModelDesc.m_ranges.resize(1);
	ModelDesc.m_ranges[0].m_ibRange = { .m_offset = 0, .m_range = VK_WHOLE_SIZE };
	ModelDesc.m_ranges[0].m_vbRange = { .m_offset = 0, .m_range = VK_WHOLE_SIZE };
	ModelDesc.m_ranges[0].m_uniformRange = { .m_offset = 0, .m_range = VK_WHOLE_SIZE };

	pPipeline->AllocateDescriptorSets((int)m_Meshes.size(), m_descriptorSets);

	pPipeline->PrepareDescriptorSets(ModelDesc, m_descriptorSets);
}


void VkModel::RecordCommandBuffer(VkCommandBuffer CmdBuf, GraphicsPipeline* pPipeline, int ImageIndex)
{
	u32 InstanceCount = 1;
	u32 FirstVertex = 0;
	u32 FirstInstance = 0;

	vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->GetPipelineLayout(),
		0,  // firstSet
		1,  // descriptorSetCount
		m_descriptorSets[ImageIndex].data(),
		0,	// dynamicOffsetCount
		NULL);	// pDynamicOffsets

	vkCmdDraw(CmdBuf, m_Meshes[0].NumIndices, InstanceCount, m_Meshes[0].BaseVertex, FirstInstance);
	//for (unsigned int i = 0; i < m_Meshes.size(); i++) {
	//	vkCmdDraw(CmdBuf, m_Meshes[i].NumIndices, InstanceCount, m_Meshes[i].BaseVertex, FirstInstance);
	//}
}


void VkModel::Update(int ImageIndex, const glm::mat4& Transformation)
{
	m_uniformBuffers[ImageIndex].Update(m_pVulkanCore->GetDevice(), &Transformation, UNIFORM_BUFFER_SIZE);
}

}