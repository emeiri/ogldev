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

	DestroyModel();

	if (m_alignedVertices.pMem) {
		free(m_alignedVertices.pMem);
	}

	if (m_alignedIndices.pMem) {
		free(m_alignedIndices.pMem);
	}	
}


Texture* VkModel::AllocTexture2D()
{
	assert(m_pVulkanCore);

	return new VulkanTexture(m_pVulkanCore);
}


void VkModel::DestroyTexture(Texture* pTexture)
{
	assert(pTexture);

	pTexture->Destroy(m_pVulkanCore->GetDevice());
}


void VkModel::PopulateBuffers(std::vector<Vertex>& Vertices)
{
	m_vertexSize = sizeof(Vertex);

	UpdateAlignedMeshesArray();

	CreateBuffers(Vertices);
}


void VkModel::UpdateAlignedMeshesArray()
{
	VkDeviceSize Alignment = m_pVulkanCore->GetPhysicalDeviceLimits().minStorageBufferOffsetAlignment;

	size_t NumSubmeshes = m_Meshes.size();

	m_alignedMeshes.resize(NumSubmeshes);

	size_t BaseVertexOffset = 0;
	size_t BaseIndexOffset = 0;

	for (int SubmeshIndex = 0; SubmeshIndex < NumSubmeshes; SubmeshIndex++) {
		m_alignedMeshes[SubmeshIndex].VertexBufferOffset = BaseVertexOffset;
		m_alignedMeshes[SubmeshIndex].VertexBufferRange = m_Meshes[SubmeshIndex].NumVertices * m_vertexSize;

		BaseVertexOffset += m_alignedMeshes[SubmeshIndex].VertexBufferRange;
		BaseVertexOffset = AlignUpToMultiple(BaseVertexOffset, Alignment);

		m_alignedMeshes[SubmeshIndex].IndexBufferOffset = BaseIndexOffset;
		m_alignedMeshes[SubmeshIndex].IndexBufferRange = m_Meshes[SubmeshIndex].NumIndices * sizeof(u32);

		BaseIndexOffset += m_alignedMeshes[SubmeshIndex].IndexBufferRange;
		BaseIndexOffset = AlignUpToMultiple(BaseIndexOffset, Alignment);
	}
}


void VkModel::CreateBuffers(std::vector<Vertex>& Vertices)
{
	size_t NumSubmeshes = m_alignedMeshes.size();

	size_t VertexBufferSize = m_alignedMeshes[NumSubmeshes - 1].VertexBufferOffset +
		                      m_alignedMeshes[NumSubmeshes - 1].VertexBufferRange;

	size_t IndexBufferSize = m_alignedMeshes[NumSubmeshes - 1].IndexBufferOffset +
		                     m_alignedMeshes[NumSubmeshes - 1].IndexBufferRange;

	assert(m_alignedVertices.pMem == NULL);
	m_alignedVertices.Size = VertexBufferSize;
	m_alignedVertices.pMem = (char*)malloc(VertexBufferSize);
	char* pSrcVertices = (char*)Vertices.data();

	assert(m_alignedIndices.pMem == NULL);
	m_alignedIndices.Size = IndexBufferSize;
	m_alignedIndices.pMem = (char*)malloc(IndexBufferSize);
	char* pSrcIndices = (char*)m_Indices.data();

	for (int SubmeshIndex = 0; SubmeshIndex < NumSubmeshes; SubmeshIndex++) {
		size_t SrcOffset = m_Meshes[SubmeshIndex].BaseVertex * m_vertexSize;
		char* pSrc = pSrcVertices + SrcOffset;
		char* pDst = m_alignedVertices.pMem + m_alignedMeshes[SubmeshIndex].VertexBufferOffset;
		size_t Size = m_alignedMeshes[SubmeshIndex].VertexBufferRange;
		memcpy(pDst, pSrc, Size);

		SrcOffset = m_Meshes[SubmeshIndex].BaseIndex * sizeof(u32);
		pSrc = pSrcIndices + SrcOffset;
		pDst = m_alignedIndices.pMem + m_alignedMeshes[SubmeshIndex].IndexBufferOffset;
		Size = m_alignedMeshes[SubmeshIndex].IndexBufferRange;
		memcpy(pDst, pSrc, Size);
	}

	m_vb = m_pVulkanCore->CreateVertexBuffer(m_alignedVertices.pMem, VertexBufferSize);

	m_ib = m_pVulkanCore->CreateVertexBuffer(m_alignedIndices.pMem, IndexBufferSize);

	m_uniformBuffers = m_pVulkanCore->CreateUniformBuffers(UNIFORM_BUFFER_SIZE * m_Meshes.size());
}


void VkModel::CreateDescriptorSets(GraphicsPipelineV2& Pipeline)
{
	int NumSubmeshes = (int)m_Meshes.size();
	Pipeline.AllocateDescriptorSets(NumSubmeshes, m_descriptorSets, m_texturesDescriptorSet);

	ModelDesc md;

	UpdateModelDesc(md);

	Pipeline.UpdateDescriptorSets(md, m_descriptorSets, m_texturesDescriptorSet);
}


void VkModel::UpdateModelDesc(ModelDesc& md)
{
	md.m_vb = m_vb.m_buffer;
	md.m_ib = m_ib.m_buffer;

	md.m_uniforms.resize(m_pVulkanCore->GetNumImages());

	for (int ImageIndex = 0; ImageIndex < m_pVulkanCore->GetNumImages(); ImageIndex++) {
		md.m_uniforms[ImageIndex] = m_uniformBuffers[ImageIndex].m_buffer;
	}

	md.m_materials.resize(m_Materials.size());

	for (int MaterialIndex = 0; MaterialIndex < m_Materials.size(); MaterialIndex++) {
		if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]) {
			Texture* pDiffuse = m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE];
			md.m_materials[MaterialIndex].m_sampler = pDiffuse->m_sampler;
			md.m_materials[MaterialIndex].m_imageView = pDiffuse->m_view;
		} else {
			printf("No diffuse texture in material %d\n", MaterialIndex);
			exit(0);
		}
	}

	md.m_ranges.resize(m_Meshes.size());

	int NumSubmeshes = (int)m_Meshes.size();

	for (int SubmeshIndex = 0; SubmeshIndex < NumSubmeshes; SubmeshIndex++) {

		size_t offset = m_alignedMeshes[SubmeshIndex].VertexBufferOffset;
		size_t range  = m_alignedMeshes[SubmeshIndex].VertexBufferRange;

		md.m_ranges[SubmeshIndex].m_vbRange = { .m_offset = offset, .m_range = range };

		offset = m_alignedMeshes[SubmeshIndex].IndexBufferOffset;
		range  = m_alignedMeshes[SubmeshIndex].IndexBufferRange;

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
		vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, 
								Pipeline.GetPipelineLayout(),
								0,  // firstSet
								1,  // descriptorSetCount
								&m_descriptorSets[ImageIndex][SubmeshIndex],
								0,	// dynamicOffsetCount
								NULL);	// pDynamicOffsets

		vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, 
								Pipeline.GetPipelineLayout(),
								1,  // firstSet
								1,  // descriptorSetCount						
								&m_texturesDescriptorSet,
								0,	// dynamicOffsetCount
								NULL);	// pDynamicOffsets

		vkCmdDraw(CmdBuf, m_Meshes[SubmeshIndex].NumIndices, 
			      InstanceCount, BaseVertex, FirstInstance);
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