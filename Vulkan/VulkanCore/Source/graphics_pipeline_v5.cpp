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

enum V4_Binding {
	V4_BindingVB = 0,
	V4_BindingIB = 1,
	V4_BindingTexture2D = 2,
	V4_BindingUniform = 3,
	V4_BindingMetaData = 4,
	V4_NumBindings = 5
};


VkDescriptorSetLayout GraphicsPipelineV5::CreateDescSetLayout(OgldevVK::VulkanCore& vkCore)
{
	std::vector<VkDescriptorSetLayoutBinding> Bindings{
		{.
		     binding = V4_BindingVB,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = V4_BindingIB,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = V4_BindingTexture2D,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_TEXTURES,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = V4_BindingUniform,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}, {
			.binding = V4_BindingMetaData,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
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

	int NumBindings = V4_NumBindings;

	std::vector<VkWriteDescriptorSet> WriteDescriptorSet(DescriptorSets.size() * NumBindings);

	std::vector<VkDescriptorBufferInfo> BufferInfo_Uniforms(DescriptorSets.size());

	for (int i = 0; i < (int)BufferInfo_Uniforms.size(); i++) {
		BufferInfo_Uniforms[i].buffer = ModelDesc.m_uniforms[i];
		BufferInfo_Uniforms[i].offset = 0;
		BufferInfo_Uniforms[i].range = VK_WHOLE_SIZE;
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

	u32 WdsIndex = 0;

	for (int i = 0; i < (int)DescriptorSets.size(); i++) {
		VkDescriptorSet& DstSet = DescriptorSets[i];

		VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = V4_BindingVB,
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
			.dstBinding = V4_BindingIB,
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
			.dstBinding = V4_BindingTexture2D,
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
			.dstBinding = V4_BindingUniform,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &BufferInfo_Uniforms[i]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = DstSet,
			.dstBinding = V4_BindingMetaData,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pImageInfo = NULL,
			.pBufferInfo = &MetaDataBufferInfo,
			.pTexelBufferView = NULL
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;
	}

	vkUpdateDescriptorSets(m_device, WdsIndex, WriteDescriptorSet.data(), 0, NULL);
}


}