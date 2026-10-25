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


namespace OgldevVK {

#define UNIFORM_BUFFER_SIZE sizeof(glm::mat4)

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

struct SubmeshMetaData {
	u32 MaterialIndex;     // index into bindless descriptor array
	u32 BaseIndex;         // offset into IndexSSBO (in indices)
	u32 NumIndices;        // number of indices for this submesh
	u32 BaseVertex;        // base vertex applied in shader
	glm::vec4 Color;
};

struct GPUMaterial {
    i32 m_baseColorTexIndex = -1;
    i32 m_normalTexIndex = -1;
    i32 m_metallicRoughnessTexIndex = -1;
    i32 m_aoTexIndex = -1;
    i32 m_emissionTexIndex = -1;
    i32 m_clearCoatTexIndex = -1;
    i32 m_clearCoatRoughnessTexIndex = -1;
    i32 m_clearCoatNormalTexIndex = -1;
    glm::vec4 m_baseColorFactor = glm::vec4(1.0f);
    glm::vec4 m_matallicRoughnessNormalAOFactor = glm::vec4(1.0f);
    glm::vec4 m_emmisiveFactorAlphaCutoff = glm::vec4(1.0f);
    glm::vec4 m_clearCoatTransmissionThinkness = glm::vec4(1.0f);
    glm::uvec4 m_typeIORAlpha = glm::uvec4(0); // x. material type, y. floatBitsToUint(IOR), z. alpha mode, w. unused
};

static_assert(sizeof(GPUMaterial) % 16 == 0, "Align GPUMaterial to 16 bytes for safety.");

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

// Matches the GLSL std430 layout (tight packing)
static_assert(sizeof(SubmeshMetaData) % 16 == 0, "Align Meta to 16 bytes for safety.");

void VkModel::Destroy()
{
	m_vb.Destroy(m_pVulkanCore->GetDevice());
	m_ib.Destroy(m_pVulkanCore->GetDevice());
	m_metaData.Destroy(m_pVulkanCore->GetDevice());
	m_indirectBuffer.Destroy(m_pVulkanCore->GetDevice());
	if (m_materialsBuffer.m_buffer != VK_NULL_HANDLE) {
		m_materialsBuffer.Destroy(m_pVulkanCore->GetDevice());
	}

	for (int i = 0; i < m_uniformBuffers.size(); i++) {
		m_uniformBuffers[i].Destroy(m_pVulkanCore->GetDevice());
	}

	DestroyModel();

	if (m_alignedVertices.pMem) {
		free(m_alignedVertices.pMem);
        m_alignedVertices.pMem = NULL;
	}

	if (m_alignedIndices.pMem) {
		free(m_alignedIndices.pMem);
        m_alignedIndices.pMem = NULL;
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

	size_t NumSubmeshes = m_modelData.m_meshes.size();

	m_alignedMeshes.resize(NumSubmeshes);

	size_t BaseVertexOffset = 0;
	size_t BaseIndexOffset = 0;

	for (int SubmeshIndex = 0; SubmeshIndex < NumSubmeshes; SubmeshIndex++) {
		m_alignedMeshes[SubmeshIndex].VertexBufferOffset = BaseVertexOffset;
		m_alignedMeshes[SubmeshIndex].VertexBufferRange = m_modelData.m_meshes[SubmeshIndex].NumVertices * m_vertexSize;

		BaseVertexOffset += m_alignedMeshes[SubmeshIndex].VertexBufferRange;
		BaseVertexOffset = AlignUpToMultiple(BaseVertexOffset, Alignment);

		m_alignedMeshes[SubmeshIndex].IndexBufferOffset = BaseIndexOffset;
		m_alignedMeshes[SubmeshIndex].IndexBufferRange = m_modelData.m_meshes[SubmeshIndex].NumIndices * sizeof(u32);

		BaseIndexOffset += m_alignedMeshes[SubmeshIndex].IndexBufferRange;
		BaseIndexOffset = AlignUpToMultiple(BaseIndexOffset, Alignment);
	}
}


void VkModel::CreateBuffers(std::vector<Vertex>& Vertices)
{
	size_t NumSubmeshes = m_alignedMeshes.size();

	// The size of the buffer can be calculated by taking the offset of the last segment
	// and adding its size.
	size_t VertexBufferSize = m_alignedMeshes[NumSubmeshes - 1].VertexBufferOffset +
		                      m_alignedMeshes[NumSubmeshes - 1].VertexBufferRange;

	size_t IndexBufferSize = m_alignedMeshes[NumSubmeshes - 1].IndexBufferOffset +
		                     m_alignedMeshes[NumSubmeshes - 1].IndexBufferRange;

	// Allocate a temporary buffer to which the segments will
	// be copied in an aligned fashion and use that to create the actual 
	// vertex buffer. Remember to deallocate the temp buffer at the end.
	assert(m_alignedVertices.pMem == NULL);
	m_alignedVertices.Size = VertexBufferSize;
	m_alignedVertices.pMem = (char*)malloc(VertexBufferSize);
	char* pSrcVertices = (char*)Vertices.data();

	assert(m_alignedIndices.pMem == NULL);
	m_alignedIndices.Size = IndexBufferSize;
	m_alignedIndices.pMem = (char*)malloc(IndexBufferSize);
	char* pSrcIndices = (char*)m_modelData.m_indices.data();

	// Copy the segments from the non aligned offsets to the aligned offsets
	// in the temporary buffer.
	for (int SubmeshIndex = 0; SubmeshIndex < NumSubmeshes; SubmeshIndex++) {
		size_t SrcOffset = m_modelData.m_meshes[SubmeshIndex].BaseVertex * m_vertexSize;
		char* pSrc = pSrcVertices + SrcOffset;
		char* pDst = m_alignedVertices.pMem + m_alignedMeshes[SubmeshIndex].VertexBufferOffset;
		size_t Size = m_alignedMeshes[SubmeshIndex].VertexBufferRange;
		memcpy(pDst, pSrc, Size);

		SrcOffset = m_modelData.m_meshes[SubmeshIndex].BaseIndex * sizeof(u32);
		pSrc = pSrcIndices + SrcOffset;
		pDst = m_alignedIndices.pMem + m_alignedMeshes[SubmeshIndex].IndexBufferOffset;
		Size = m_alignedMeshes[SubmeshIndex].IndexBufferRange;
		memcpy(pDst, pSrc, Size);
	}

	m_vb = m_pVulkanCore->CreateVertexBuffer(m_alignedVertices.pMem, VertexBufferSize);

	m_ib = m_pVulkanCore->CreateVertexBuffer(m_alignedIndices.pMem, IndexBufferSize);

	if (m_useInternalUniformBuffers) {
		m_uniformBuffers = m_pVulkanCore->CreateUniformBuffers(UNIFORM_BUFFER_SIZE * m_modelData.m_meshes.size());
	}

	if (m_isDescriptorIndexing) {
		CreateMetaData();
		CreateIndirectBuffer();
	}
}


void VkModel::CreateMetaData()
{
	size_t NumSubmeshes = m_alignedMeshes.size();

	std::vector<SubmeshMetaData> MetaData(NumSubmeshes);

	for (int SubmeshIndex = 0; SubmeshIndex < NumSubmeshes; SubmeshIndex++) {
        int MaterialIndex = m_modelData.m_meshes[SubmeshIndex].MaterialIndex;
		MetaData[SubmeshIndex].MaterialIndex = MaterialIndex;
		MetaData[SubmeshIndex].BaseIndex = (u32)m_alignedMeshes[SubmeshIndex].IndexBufferOffset;
		MetaData[SubmeshIndex].NumIndices = m_modelData.m_meshes[SubmeshIndex].NumIndices;
		MetaData[SubmeshIndex].BaseVertex = (u32)m_alignedMeshes[SubmeshIndex].VertexBufferOffset;

		CoreMaterial* pMaterial = &m_Materials[MaterialIndex];
        MetaData[SubmeshIndex].Color = { pMaterial->m_pMaterialData->m_baseColor.r, 
										 pMaterial->m_pMaterialData->m_baseColor.g, 
										 pMaterial->m_pMaterialData->m_baseColor.b, 
										 pMaterial->m_pMaterialData->m_baseColor.a };
	}

	m_metaData = m_pVulkanCore->CreateSSBO(MetaData.data(), ARRAY_SIZE_IN_BYTES(MetaData));
}


void VkModel::CreateIndirectBuffer()
{
	std::vector<VkDrawIndirectCommand> DrawCommands(m_modelData.m_meshes.size());

	for (u32 SubmeshIndex = 0; SubmeshIndex < m_modelData.m_meshes.size(); SubmeshIndex++) {
		VkDrawIndirectCommand cmd = {
			.vertexCount = m_modelData.m_meshes[SubmeshIndex].NumIndices,
			.instanceCount = 1,
			.firstVertex = 0,
			.firstInstance = SubmeshIndex
		};

		DrawCommands[SubmeshIndex] = cmd;
	}

	m_indirectBuffer = m_pVulkanCore->CreateIndirectBuffer(DrawCommands.data(), ARRAY_SIZE_IN_BYTES(DrawCommands));
}


void VkModel::CreateDescriptorSets(GraphicsPipelineV2& Pipeline)
{
	assert(!m_isDescriptorIndexing);

	int NumSubmeshes = (int)m_modelData.m_meshes.size();
	Pipeline.AllocateDescriptorSets(NumSubmeshes, m_descriptorSets);

	ModelDesc md;

	UpdateModelDesc(md);

	Pipeline.UpdateDescriptorSets(md, m_descriptorSets);
}


void VkModel::CreateDescriptorSets(GraphicsPipelineV3& Pipeline)
{
	assert(m_isDescriptorIndexing);

	int NumSubmeshes = (int)m_modelData.m_meshes.size();
	Pipeline.AllocateDescriptorSets(NumSubmeshes, m_descriptorSets, m_texturesDescriptorSet);

	ModelDesc md;

	UpdateModelDesc(md);

	Pipeline.UpdateDescriptorSets(md, m_descriptorSets, m_texturesDescriptorSet);
}


void VkModel::CreateDescriptorSets(GraphicsPipelineV4& Pipeline)
{
	assert(m_isDescriptorIndexing);

	m_descriptorSets.resize(1);
	Pipeline.AllocateDescriptorSets(m_descriptorSets[0]);

	ModelDesc md;

	UpdateModelDesc(md);

	Pipeline.UpdateDescriptorSets(md, m_descriptorSets[0]);
}


void VkModel::UpdateModelDesc(ModelDesc& md)
{
	md.m_vb = m_vb.m_buffer;
	md.m_ib = m_ib.m_buffer;
	md.m_metaData = m_metaData.m_buffer;    

	md.m_uniforms.resize(m_pVulkanCore->GetNumImages());

	if (m_useInternalUniformBuffers) {
		for (int ImageIndex = 0; ImageIndex < m_pVulkanCore->GetNumImages(); ImageIndex++) {
			md.m_uniforms[ImageIndex] = m_uniformBuffers[ImageIndex].m_buffer;
		}
	}

	if (m_isDescriptorIndexing) {

		std::vector<GPUMaterial> GPU_Materials(m_Materials.size());

		for (int MaterialIndex = 0; MaterialIndex < m_Materials.size(); MaterialIndex++) {
			if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]) {
				Texture* pDiffuse = m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE];

                TextureInfo ti;
                ti.m_sampler = pDiffuse->m_sampler;
                ti.m_imageView = pDiffuse->m_view;
                md.m_materials.push_back(ti);
				
                int TextureIndex = (int)md.m_materials.size() - 1;
                GPU_Materials[MaterialIndex].m_baseColorTexIndex = TextureIndex; // baseColor
			} else {
				printf("No diffuse texture in material %d\n", MaterialIndex);
				exit(0);
			}

			if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_NORMAL]) {
				Texture* pNormal = m_Materials[MaterialIndex].pTextures[TEX_TYPE_NORMAL];
				md.m_materials.push_back({ pNormal->m_sampler, pNormal->m_view });

				int TextureIndex = (int)md.m_materials.size() - 1;
				GPU_Materials[MaterialIndex].m_normalTexIndex = TextureIndex; // normal
			}

			if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_METALNESS]) {
				Texture* pMetalness = m_Materials[MaterialIndex].pTextures[TEX_TYPE_METALNESS];
				md.m_materials.push_back({ pMetalness->m_sampler, pMetalness->m_view });

				int TextureIndex = (int)md.m_materials.size() - 1;
				GPU_Materials[MaterialIndex].m_metallicRoughnessTexIndex = TextureIndex; // metalness
			} else if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_ROUGHNESS]) {
				assert(0 && "Metalness and roughness textures must be combined into a single metallic-roughness texture for PBR.");
				Texture* pRoughness = m_Materials[MaterialIndex].pTextures[TEX_TYPE_ROUGHNESS];
				md.m_materials.push_back({ pRoughness->m_sampler, pRoughness->m_view });
				int TextureIndex = (int)md.m_materials.size() - 1;
				GPU_Materials[MaterialIndex].m_metallicRoughnessTexIndex = TextureIndex; // roughness
			}

            if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_AMBIENT_OCCLUSION]) {
                Texture* pAO = m_Materials[MaterialIndex].pTextures[TEX_TYPE_AMBIENT_OCCLUSION];
                md.m_materials.push_back({ pAO->m_sampler, pAO->m_view });
                int TextureIndex = (int)md.m_materials.size() - 1;
                GPU_Materials[MaterialIndex].m_aoTexIndex = TextureIndex; // ao
            }

            if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_EMISSIVE]) {
                Texture* pEmissive = m_Materials[MaterialIndex].pTextures[TEX_TYPE_EMISSIVE];
                md.m_materials.push_back({ pEmissive->m_sampler, pEmissive->m_view });
                int TextureIndex = (int)md.m_materials.size() - 1;
                GPU_Materials[MaterialIndex].m_emissionTexIndex = TextureIndex; // emission
            }

            const Vector4f& BaseColor = m_Materials[MaterialIndex].m_pMaterialData->m_baseColor;
            GPU_Materials[MaterialIndex].m_baseColorFactor = BaseColor.ToGLM();

            const Vector4f& mrno = m_Materials[MaterialIndex].m_pMaterialData->m_metallicRoughnessNormalOcclusion;
            GPU_Materials[MaterialIndex].m_matallicRoughnessNormalAOFactor = mrno.ToGLM();

            const Vector4f& Emissive = m_Materials[MaterialIndex].m_pMaterialData->m_emissiveColor;
            GPU_Materials[MaterialIndex].m_emmisiveFactorAlphaCutoff = Emissive.ToGLM();

            const Vector4f& ClearCoat = m_Materials[MaterialIndex].m_pMaterialData->m_clearCoatTransmissionThickness;
            GPU_Materials[MaterialIndex].m_clearCoatTransmissionThinkness = ClearCoat.ToGLM();

            GPU_Materials[MaterialIndex].m_typeIORAlpha = glm::uvec4(
                m_Materials[MaterialIndex].m_pMaterialData->m_materialType,
                glm::floatBitsToUint(m_Materials[MaterialIndex].m_pMaterialData->m_ior),
                m_Materials[MaterialIndex].m_pMaterialData->m_alphaTest,
                0);
		}

        if (m_materialsBuffer.m_buffer == VK_NULL_HANDLE) {
			m_materialsBuffer = m_pVulkanCore->CreateSSBO(GPU_Materials.data(), ARRAY_SIZE_IN_BYTES(GPU_Materials));
			md.m_materialsData = m_materialsBuffer.m_buffer;
        }		
	} else {
		md.m_materials.resize(m_modelData.m_meshes.size());
	}

	md.m_ranges.resize(m_modelData.m_meshes.size());

	int NumSubmeshes = (int)m_modelData.m_meshes.size();

	for (int SubmeshIndex = 0; SubmeshIndex < NumSubmeshes; SubmeshIndex++) {
		if (!m_isDescriptorIndexing) {
			int MaterialIndex = m_modelData.m_meshes[SubmeshIndex].MaterialIndex;

			if ((MaterialIndex >= 0) && (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE])) {
				Texture* pDiffuse = m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE];
				md.m_materials[SubmeshIndex].m_sampler = pDiffuse->m_sampler;
				md.m_materials[SubmeshIndex].m_imageView = pDiffuse->m_view;
			} else {
				printf("No diffuse texture in material %d\n", MaterialIndex);
				exit(0);
			}
		}

		size_t offset = m_alignedMeshes[SubmeshIndex].VertexBufferOffset;
		size_t range = m_alignedMeshes[SubmeshIndex].VertexBufferRange;

		md.m_ranges[SubmeshIndex].m_vbRange = { .m_offset = offset, .m_range = range };

		offset = m_alignedMeshes[SubmeshIndex].IndexBufferOffset;
		range = m_alignedMeshes[SubmeshIndex].IndexBufferRange;

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

	for (u32 SubmeshIndex = 0; SubmeshIndex < m_modelData.m_meshes.size(); SubmeshIndex++) {
		vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, 
								Pipeline.GetPipelineLayout(),
								0,  // firstSet
								1,  // descriptorSetCount
								&m_descriptorSets[ImageIndex][SubmeshIndex],
								0,	// dynamicOffsetCount
								NULL);	// pDynamicOffsets

		vkCmdDraw(CmdBuf, m_modelData.m_meshes[SubmeshIndex].NumIndices, 
			      InstanceCount, BaseVertex, FirstInstance);
	}
}


void VkModel::RecordCommandBuffer(VkCommandBuffer CmdBuf, GraphicsPipelineV3& Pipeline, int ImageIndex)
{
	u32 InstanceCount = 1;
	u32 BaseVertex = 0;

	vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
							Pipeline.GetPipelineLayout(),
							1,      // firstSet
							1,      // descriptorSetCount						
							&m_texturesDescriptorSet,
							0,	    // dynamicOffsetCount
							NULL);	// pDynamicOffsets


	for (u32 SubmeshIndex = 0; SubmeshIndex < m_modelData.m_meshes.size(); SubmeshIndex++) {
		vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
								Pipeline.GetPipelineLayout(),
								0,      // firstSet
								1,      // descriptorSetCount
								&m_descriptorSets[ImageIndex][SubmeshIndex],
								0,	    // dynamicOffsetCount
								NULL);	// pDynamicOffsets

		vkCmdDraw(CmdBuf, m_modelData.m_meshes[SubmeshIndex].NumIndices,
			      InstanceCount, BaseVertex, SubmeshIndex);
	}
}


void VkModel::RecordCommandBufferIndirect(VkCommandBuffer CmdBuf, VkPipelineLayout PipelineLayout, int ImageIndex)
{
	vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		PipelineLayout,
		0,      // firstSet
		1,      // descriptorSetCount						
		&m_descriptorSets[0][ImageIndex],
		0,	    // dynamicOffsetCount
		NULL);	// pDynamicOffsets

	RecordCommandBufferIndirect(CmdBuf);
}


void VkModel::RecordCommandBufferIndirect(VkCommandBuffer CmdBuf)
{
	vkCmdDrawIndirect(CmdBuf,
					  m_indirectBuffer.m_buffer,
					  0,  // offset inside the indirect buffer            
					  (u32)m_modelData.m_meshes.size(), // number of VkDrawIndirectCommand elements
					  sizeof(VkDrawIndirectCommand)); // stride between VkDrawIndirectCommand elements
}


void VkModel::Update(int ImageIndex, const glm::mat4& Transformation)
{
    if (!m_useInternalUniformBuffers) {
        printf("VkModel::Update called but m_useInternalUniformBuffers is false\n");
        assert(0);
    }

	std::vector<glm::mat4> Transformations(m_modelData.m_meshes.size());

	for (u32 SubmeshIndex = 0; SubmeshIndex < Transformations.size(); SubmeshIndex++) {
		Transformations[SubmeshIndex] = Transformation * GetTransformations()[SubmeshIndex];
	}

	m_uniformBuffers[ImageIndex].Update(m_pVulkanCore->GetDevice(), Transformations.data(), ARRAY_SIZE_IN_BYTES(Transformations));
}


void VkModel::GetWindowSize(int& Width, int& Height) const
{
	GLFWwindow* pWindow = m_pVulkanCore->GetWindow();
	glfwGetFramebufferSize(pWindow, &Width, &Height);
}

}