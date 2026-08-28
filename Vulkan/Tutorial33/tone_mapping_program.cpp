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

#include <stdio.h>

#include "ogldev_types.h"
#include "ogldev_util.h"
#include "ogldev_vulkan_util.h"
#include "tone_mapping_program.h"
#include "ogldev_vulkan_shader.h"


namespace OgldevVK {


void ToneMappingProgram::Init(VulkanCore& vkCore, VkDescriptorPool DescPool)
{	
	m_numImages = vkCore.GetNumImages();

	m_vs = OgldevVK::CreateShaderModuleFromText(vkCore.GetDevice(), "full_screen_tri.vert");

	m_fs = OgldevVK::CreateShaderModuleFromText(vkCore.GetDevice(), "tone_map.frag");

	GraphicsPipeline::Init(vkCore, DescPool, m_vs, m_fs, NULL, NULL);
}


void ToneMappingProgram::AllocDescSets(std::vector<VkDescriptorSet>& DescSets)
{
	GraphicsPipeline::AllocDescSets(m_numImages, DescSets, 0);
}


void ToneMappingProgram::Destroy()
{
	vkDestroyShaderModule(m_device, m_vs, NULL);
	vkDestroyShaderModule(m_device, m_fs, NULL);

	GraphicsPipeline::Destroy(true);
}


void ToneMappingProgram::Bind(VkCommandBuffer CmdBuf, VkDescriptorSet& DescSet)
{
	std::vector<VkDescriptorSet> DescSets = { DescSet };
	GraphicsPipeline::Bind(CmdBuf, DescSets);

    u32 PushConstants[2] = { 0, 0 }; // Exposure and tone mapping mode

	vkCmdPushConstants(CmdBuf,
		m_pipelineLayout,
		VK_SHADER_STAGE_FRAGMENT_BIT, // Must match shader stage
		0,                            // Offset in push constant block
		sizeof(u32) * 2,				  // Size of data
		&PushConstants[0]);
}


std::vector<VkDescriptorSetLayout> ToneMappingProgram::CreateDescSetLayout(OgldevVK::VulkanCore& vkCore)
{
	std::vector<VkDescriptorSetLayoutBinding> Bindings {
	   {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = NULL
	   }
	};

	VkDescriptorSetLayout l = vkCore.CreateDescSetLayout({ Bindings });

	std::vector<VkDescriptorSetLayout> ret = { l };

	return ret;
}


std::vector<VkPushConstantRange> ToneMappingProgram::GetPushConstantRange()
{
	std::vector<VkPushConstantRange> PushConstantRanges = {
		{
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.offset = 0,
            .size = sizeof(u32) * 2 // For exposure and tone mapping mode
		}
	};

	return PushConstantRanges;
}


void ToneMappingProgram::UpdateDescriptorSets(std::vector<VkDescriptorSet>& DescriptorSets,
											  const std::vector<VkImageView>& SwapChainImageViews,
											  const std::vector<OfflineImage>& OfflineImages)
{
	int DescCount = (int)DescriptorSets.size();
	std::vector<VkWriteDescriptorSet> WriteDescriptorSet(DescCount);

	// Allocate space for each image's layout parameters
	std::vector<VkDescriptorImageInfo> InputInfo(DescCount);

	u32 WdsIndex = 0;

	for (int i = 0; i < DescCount; i++) {
		VkDescriptorSet& DstSet = DescriptorSets[i];

		InputInfo[i] = {
            .sampler = OfflineImages[i].m_color.m_sampler,
			.imageView = OfflineImages[i].m_color.m_view,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};
		
		// Write the Storage Image Descriptor to binding 0
		VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &InputInfo[i]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;	
	}

	vkUpdateDescriptorSets(m_device, WdsIndex, WriteDescriptorSet.data(), 0, NULL);
}


void ToneMappingProgram::RecordCommandBuffer(VkCommandBuffer CmdBuf)
{
	u32 VertexCount = 3;
	u32 InstanceCount = 1;
	u32 BaseVertex = 0;
	u32 FirstInstance = 0;

	vkCmdDraw(CmdBuf, VertexCount, InstanceCount, BaseVertex, FirstInstance);
}

}