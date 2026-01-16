/*
		Copyright 2026 Etay Meiri

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

#include <assert.h>

#include "Pipelines/ogldev_vulkan_texgen_pipeline.h"

namespace OgldevVK {

VkDescriptorSetLayout TexGenComputePipeline::CreateDescSetLayout(OgldevVK::VulkanCore& vkCore)
{
	std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

	VkDescriptorSetLayoutBinding Binding = {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.pImmutableSamplers = NULL
	};

	LayoutBindings.push_back(Binding);

	VkDescriptorSetLayoutBinding Binding_Uniform = {
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
	};

	LayoutBindings.push_back(Binding_Uniform);

	return vkCore.CreateDescSetLayout(LayoutBindings);
}


void TexGenComputePipeline::UpdateDescSets(std::vector<VkDescriptorSet>& DescriptorSets, 
										   const VulkanTexture& Texture, const std::vector<BufferAndMemory>& UBOs)
{
	std::vector<VkWriteDescriptorSet> WriteDescriptorSet(m_numImages * 2);

	std::vector<VkDescriptorBufferInfo> BufferInfo_Uniforms(m_numImages);

	for (int ImageIndex = 0; ImageIndex < m_numImages; ImageIndex++) {
		BufferInfo_Uniforms[ImageIndex].buffer = UBOs[ImageIndex].m_buffer;
		BufferInfo_Uniforms[ImageIndex].offset = 0;
		BufferInfo_Uniforms[ImageIndex].range = VK_WHOLE_SIZE;
	}

	u32 WdsIndex = 0;

	VkDescriptorImageInfo ImageInfo = {
		.sampler = Texture.m_sampler,
		.imageView = Texture.m_view,
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL
	};

	for (int ImageIndex = 0; ImageIndex < m_numImages; ImageIndex++) {
		VkDescriptorSet& DstSet = DescriptorSets[ImageIndex];

		VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.pImageInfo = &ImageInfo
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = 1,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &BufferInfo_Uniforms[ImageIndex]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;
	}

	vkUpdateDescriptorSets(m_device, WdsIndex, WriteDescriptorSet.data(), 0, NULL);
}


}