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

#include "postprocess_compute_pipeline.h"

VkDescriptorSetLayout PostprocessComputePipeline::CreateDescSetLayout(OgldevVK::VulkanCore& vkCore)
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


void PostprocessComputePipeline::UpdateDescSets(std::vector<VkDescriptorSet>& DescriptorSets,
                                                const std::vector<VkImageView>& SwapChainImageViews)
{
    // We only have 1 descriptor write per swapchain image now
    std::vector<VkWriteDescriptorSet> WriteDescriptorSet(m_numImages);

    // Allocate space for each image's layout parameters
    std::vector<VkDescriptorImageInfo> ImageInfos(m_numImages);

    u32 WdsIndex = 0;

    for (int ImageIndex = 0; ImageIndex < m_numImages; ImageIndex++) {
        VkDescriptorSet& DstSet = DescriptorSets[ImageIndex];

        // Configure the descriptor to point to this specific frame's swapchain view
        ImageInfos[ImageIndex] = {
            .sampler = VK_NULL_HANDLE,           // Storage images do not use a sampler
            .imageView = SwapChainImageViews[ImageIndex],
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL // Matches your Synchronization2 barrier layout
        };

        // Write the Storage Image Descriptor to binding 0
        VkWriteDescriptorSet wds = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = DstSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &ImageInfos[ImageIndex]
        };

        assert(WdsIndex < WriteDescriptorSet.size());
        WriteDescriptorSet[WdsIndex++] = wds;
    }

    vkUpdateDescriptorSets(m_device, WdsIndex, WriteDescriptorSet.data(), 0, NULL);
}
