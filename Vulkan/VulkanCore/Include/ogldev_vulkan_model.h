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

#include "Int/core_model.h"
#include "ogldev_vulkan_texture.h"
#include "Int/model_desc.h"

namespace OgldevVK {

class VulkanCore;
class GraphicsPipelineV2;

class VkModel : public CoreModel
{
public:

	VkModel() {}

	void Destroy();

	void Init(VulkanCore* pVulkanCore) { m_pVulkanCore = pVulkanCore; }

	void CreateDescriptorSets(GraphicsPipelineV2* pPipeline);

	void RecordCommandBuffer(VkCommandBuffer CmdBuf, GraphicsPipelineV2* pPipeline, int ImageIndex);

	void Update(int ImageIndex, const glm::mat4& Transformation);

	virtual void SetColorTexture(int TextureHandle) { assert(0); }

	virtual void SetNormalMap(int TextureHandle) { assert(0); }

	virtual void SetHeightMap(int TextureHandle) { assert(0); }

	virtual void SetTextureScale(float Scale) { assert(0); }

	const BufferAndMemory* GetVB() const { return &m_vb; }

	const BufferAndMemory* GetIB() const { return &m_ib; }

protected:

	virtual void AllocBuffers() { /* Nothing to do here */ }

	virtual Texture* AllocTexture2D();

	virtual void InitGeometryPost() { /* Nothing to do here */ }

	virtual void PopulateBuffersSkinned(vector<SkinnedVertex>& Vertices) { assert(0); }

	virtual void PopulateBuffers(vector<Vertex>& Vertices);

private:
	void CreateModelDescriptor(ModelDesc& md);

	VulkanCore* m_pVulkanCore = NULL;

	BufferAndMemory m_vb;
	BufferAndMemory m_ib;
	std::vector<OgldevVK::BufferAndMemory> m_uniformBuffers;
	std::vector<std::vector<VkDescriptorSet>> m_descriptorSets;
	size_t m_vertexSize = 0;
};

}
