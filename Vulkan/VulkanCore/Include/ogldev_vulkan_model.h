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

#pragma once

#include <vector>

#include "ogldev_vulkan_texture.h"
#include "ogldev_vulkan_graphics_pipeline_v2.h"
#include "Int/core_model.h"
#include "Int/model_desc.h"

namespace OgldevVK {

class VulkanCore;

class VkModel : public CoreModel
{
public:

	VkModel() {}

	void Destroy();

	void Init(VulkanCore* pVulkanCore) { m_pVulkanCore = pVulkanCore; }

	virtual void ConvertToMesh(MeshData& mesh) { assert(0); }

	void CreateDescriptorSets(GraphicsPipelineV2& Pipeline);

	void RecordCommandBuffer(VkCommandBuffer CmdBuf, GraphicsPipelineV2& pPipeline, int ImageIndex);

	void Update(int ImageIndex, const glm::mat4& Transformation);

	const BufferAndMemory* GetVB() const { return &m_vb; }

	const BufferAndMemory* GetIB() const { return &m_ib; }

protected:

	virtual void AllocBuffers() { /* Nothing to do here */ }

	virtual Texture* AllocTexture2D();

	virtual void DestroyTexture(Texture* pTexture);

	virtual void InitGeometryPost() { /* Nothing to do here */ }

	virtual void PopulateBuffersSkinned(std::vector<SkinnedVertex>& Vertices) { assert(0); }

	virtual void PopulateBuffers(std::vector<Vertex>& Vertices);

private:
	void UpdateModelDesc(ModelDesc& md);

	void UpdateAlignedMeshesArray();

	void CreateBuffers(std::vector<Vertex>& Vertices);

	VulkanCore* m_pVulkanCore = NULL;

	BufferAndMemory m_vb;
	BufferAndMemory m_ib;
	std::vector<BufferAndMemory> m_uniformBuffers;
	std::vector<std::vector<VkDescriptorSet>> m_descriptorSets;
	size_t m_vertexSize = 0;	// sizeof(Vertex) OR sizeof(SkinnedVertex)

	struct VulkanMeshEntry {
		size_t VertexBufferOffset = 0;
		size_t VertexBufferRange = 0;
		size_t IndexBufferOffset = 0;
		size_t IndexBufferRange = 0;
	};

	std::vector<VulkanMeshEntry> m_alignedMeshes;

	MemBuf m_alignedVertices;
	MemBuf m_alignedIndices;
};

}
