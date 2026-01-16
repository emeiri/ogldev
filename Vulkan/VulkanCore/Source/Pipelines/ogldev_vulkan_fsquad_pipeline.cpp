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

#pragma once


#include "Pipelines/ogldev_vulkan_fsquad_pipeline.h"

namespace OgldevVK {

VkDescriptorSetLayout FullScreenQuadProgram::CreateDescSetLayout(OgldevVK::VulkanCore& vkCore)
{
	VkDescriptorSetLayoutBinding Binding = {
		.binding = SamplerBinding,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = NULL
	};

	return vkCore.CreateDescSetLayout({ Binding });
}


void FullScreenQuadProgram::UpdateDescSets(std::vector<VkDescriptorSet>& DescriptorSets, const OgldevVK::VulkanTexture& Tex)
{
	std::vector<VkWriteDescriptorSet> WriteDescriptorSet(DescriptorSets.size());

	VkDescriptorImageInfo ImageInfo = {
		.sampler = Tex.m_sampler,
		.imageView = Tex.m_view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	for (int i = 0; i < (int)DescriptorSets.size(); i++) {
		VkDescriptorSet& DstSet = DescriptorSets[i];

		WriteDescriptorSet[i] = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = DstSet,
			.dstBinding = SamplerBinding,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &ImageInfo,
			.pBufferInfo = NULL,
			.pTexelBufferView = NULL
		};
	}

	vkUpdateDescriptorSets(m_device, (u32)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
}


void FullScreenQuadProgram::RecordCommandBuffer(VkCommandBuffer CmdBuf)
{
	u32 VertexCount = 3;
	u32 InstanceCount = 1;
	u32 FirstVertex = 0;
	u32 FirstInstance = 0;

	vkCmdDraw(CmdBuf, VertexCount, InstanceCount, FirstVertex, FirstInstance);
}


}