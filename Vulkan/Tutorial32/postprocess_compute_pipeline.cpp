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
	std::vector<VkDescriptorSetLayoutBinding> LayoutBindings(2);

    // Binding 0: For the input Offline Image
    LayoutBindings[0].binding = 0;
    LayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    LayoutBindings[0].descriptorCount = 1;
    LayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    // Binding 1: For the output Swapchain Image
    LayoutBindings[1].binding = 1;
    LayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    LayoutBindings[1].descriptorCount = 1;
    LayoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	return vkCore.CreateDescSetLayout(LayoutBindings);
}


void PostprocessComputePipeline::UpdateDescSets(std::vector<VkDescriptorSet>& DescriptorSets,
                                                const std::vector<OgldevVK::VulkanBaseImage>& SwapChainImages,
                                                const std::vector<OfflineImage>& OfflineImages)
{
    // We only have 1 descriptor write per swapchain image now
    int DescCount = (int)DescriptorSets.size();
    std::vector<VkWriteDescriptorSet> WriteDescriptorSet(DescCount * 2); // Two writes per descriptor set (input and output images)

    // Allocate space for each image's layout parameters
    std::vector<VkDescriptorImageInfo> InputInfo(DescCount);
    std::vector<VkDescriptorImageInfo> OutputInfo(DescCount);

    u32 WdsIndex = 0;

    for (int i = 0; i < DescCount; i++) {
        VkDescriptorSet& DstSet = DescriptorSets[i];

        InputInfo[i] = {
            .sampler = VK_NULL_HANDLE,           // Storage images do not use a sampler
            .imageView = OfflineImages[i].m_color.m_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL // Matches your Synchronization2 barrier layout
        };

        OutputInfo[i] = {
            .sampler = VK_NULL_HANDLE,           // Storage images do not use a sampler
            .imageView = SwapChainImages[i].m_view,
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
            .pImageInfo = &InputInfo[i]
        };

        assert(WdsIndex < WriteDescriptorSet.size());
        WriteDescriptorSet[WdsIndex++] = wds;

        wds = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = DstSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &OutputInfo[i]
        };

        assert(WdsIndex < WriteDescriptorSet.size());
        WriteDescriptorSet[WdsIndex++] = wds;
    }

    vkUpdateDescriptorSets(m_device, WdsIndex, WriteDescriptorSet.data(), 0, NULL);
}
