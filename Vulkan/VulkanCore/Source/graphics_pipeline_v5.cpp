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

#include <stdio.h>

#include "ogldev_types.h"
#include "ogldev_util.h"
#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_graphics_pipeline_v5.h"


namespace OgldevVK {

const u32 MAX_TEXTURES = 4096; // choose according to limits and memory

enum V5_Binding {
	V5_BindingVB = 0,
	V5_BindingIB = 1,
	V5_BindingTexture2D = 2,
	V5_BindingUniformVS = 3,
	V5_BindingMetaData = 4,
	V5_BindingUniformFS = 5,
	V5_NumBindings = 6
};


struct UniformDataFS {
	glm::vec4 AmbientLight;      // .rgb = color, .w = intensity
	glm::vec4 LightDirection;    // .xyz = direction
	glm::vec4 LightColor;        // .rgb = color, .w = intensity
};


struct UniformDataVS {
	glm::mat4 WVP;
	glm::mat4 NormalMatrix;
};


#define MAX_NUM_MESHES (64 * 1024 / sizeof(UniformDataVS))

void GraphicsPipelineV5::Init(VulkanCore& vkCore, VkDescriptorPool DescPool, VkShaderModule vs, VkShaderModule fs)
{
	u32 renderModeValue = 1;

	VkSpecializationMapEntry SpecMapEntry = {
		.constantID = 0,
		.offset = 0,
		.size = sizeof(u32)
	};

	VkSpecializationInfo SpecInfo = {
		.mapEntryCount = 1,
		.pMapEntries = &SpecMapEntry,
		.dataSize = sizeof(u32),
		.pData = &renderModeValue
	};

	GraphicsPipeline::Init(vkCore, DescPool, vs, fs, NULL, &SpecInfo);

	m_uniformBuffersVS = vkCore.CreateUniformBuffers(MAX_NUM_MESHES * sizeof(UniformDataVS));
	m_uniformBuffersFS = vkCore.CreateUniformBuffers(sizeof(UniformDataFS));
}


void GraphicsPipelineV5::Destroy()
{
	GraphicsPipeline::Destroy();

	for (int i = 0; i < m_uniformBuffersVS.size(); i++) {
		m_uniformBuffersVS[i].Destroy(m_device);
		m_uniformBuffersFS[i].Destroy(m_device);
	}
}


VkDescriptorSetLayout GraphicsPipelineV5::CreateDescSetLayout(OgldevVK::VulkanCore& vkCore)
{
	std::vector<VkDescriptorSetLayoutBinding> Bindings{
		{.
		     binding = V5_BindingVB,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = V5_BindingIB,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = V5_BindingTexture2D,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_TEXTURES,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = V5_BindingUniformVS,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = V5_BindingMetaData,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = V5_BindingUniformFS,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		}
	};

	VkDescriptorSetLayout l = vkCore.CreateDescSetLayout({ Bindings });

	return l;
}


void GraphicsPipelineV5::UpdateDescriptorSets(const ModelDesc& ModelDesc,
											  std::vector<VkDescriptorSet>& DescriptorSets)
{
	assert(DescriptorSets.size() != 0);

	std::vector<VkDescriptorBufferInfo> BufferInfoUniformsVS(DescriptorSets.size());

	for (int i = 0; i < (int)BufferInfoUniformsVS.size(); i++) {
		BufferInfoUniformsVS[i].buffer = m_uniformBuffersVS[i].m_buffer;
		BufferInfoUniformsVS[i].offset = 0;
		BufferInfoUniformsVS[i].range = VK_WHOLE_SIZE;
	}

	std::vector<VkDescriptorBufferInfo> BufferInfoUniformsFS(DescriptorSets.size());

	for (int i = 0; i < (int)BufferInfoUniformsFS.size(); i++) {
		BufferInfoUniformsFS[i].buffer = m_uniformBuffersFS[i].m_buffer;
		BufferInfoUniformsFS[i].offset = 0;
		BufferInfoUniformsFS[i].range = VK_WHOLE_SIZE;
	}

	u32 TextureCount = (u32)ModelDesc.m_materials.size();
	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(TextureCount);

	for (u32 i = 0; i < TextureCount; ++i) {
		ImageInfos[i].sampler = ModelDesc.m_materials[i].m_sampler;
		ImageInfos[i].imageView = ModelDesc.m_materials[i].m_imageView;
		ImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	VkDescriptorBufferInfo BufferInfo_VB = {
		.buffer = ModelDesc.m_vb,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo BufferInfo_IB = {
		.buffer = ModelDesc.m_ib,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};
	
	VkDescriptorBufferInfo MetaDataBufferInfo = {
		.buffer = ModelDesc.m_metaData,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	std::vector<VkWriteDescriptorSet> WriteDescriptorSet(DescriptorSets.size() * V5_NumBindings);

	u32 WdsIndex = 0;

	for (int i = 0; i < (int)DescriptorSets.size(); i++) {
		VkDescriptorSet& DstSet = DescriptorSets[i];

		VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = V5_BindingVB,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &BufferInfo_VB
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = V5_BindingIB,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &BufferInfo_IB
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = DstSet,
			.dstBinding = V5_BindingTexture2D,
			.dstArrayElement = 0,
			.descriptorCount = TextureCount,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = ImageInfos.data(),
			.pBufferInfo = NULL,
			.pTexelBufferView = NULL
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = V5_BindingUniformVS,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &BufferInfoUniformsVS[i]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = DstSet,
			.dstBinding = V5_BindingMetaData,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pImageInfo = NULL,
			.pBufferInfo = &MetaDataBufferInfo,
			.pTexelBufferView = NULL
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = V5_BindingUniformFS,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &BufferInfoUniformsFS[i]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;
	}

	vkUpdateDescriptorSets(m_device, WdsIndex, WriteDescriptorSet.data(), 0, NULL);
}


void GraphicsPipelineV5::UpdateUniformBuffers(int ImageIndex, 
											  const glm::mat4& WVP, 
											  const glm::mat4& World,				
											  const std::vector<glm::mat4>& SubmeshTransformations,
											  const glm::vec4& AmbientLight)
{
	std::vector<UniformDataVS> UboDataVS(SubmeshTransformations.size());

	for (int i = 0; i < SubmeshTransformations.size(); i++) {
		UboDataVS[i].WVP = WVP * SubmeshTransformations[i];

		glm::mat4 WorldAndBase = World * SubmeshTransformations[i];
		glm::mat3 NormalMatrix = glm::transpose(glm::inverse(glm::mat3(WorldAndBase)));

		UboDataVS[i].NormalMatrix = NormalMatrix;
	}

	m_uniformBuffersVS[ImageIndex].Update(m_device, UboDataVS.data(), sizeof(UniformDataVS) * UboDataVS.size());

	UniformDataFS UboDataFS = {
		.AmbientLight = AmbientLight,
		.LightDirection = glm::vec4(1.0f, -1.0f, 0.0f, 0.0f),
		.LightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
	};

	m_uniformBuffersFS[ImageIndex].Update(m_device, &UboDataFS, sizeof(UboDataFS));	
}


}