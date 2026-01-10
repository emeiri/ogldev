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

#include "ogldev_vulkan_compute_pipeline.h"

namespace OgldevVK {

ComputePipeline::ComputePipeline(VulkanCore& vkCore, VkShaderModule cs)
{
	m_device = vkCore.GetDevice();
	m_numImages = vkCore.GetNumImages();

	std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;	

	VkDescriptorSetLayoutBinding Binding = {
		.binding = 2,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.pImmutableSamplers = NULL
	};

	LayoutBindings.push_back(Binding);

	VkDescriptorSetLayoutBinding Binding_Uniform = {
		.binding = 3,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
	};

	LayoutBindings.push_back(Binding_Uniform);

	VkDescriptorSetLayoutCreateInfo LayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.bindingCount = (u32)LayoutBindings.size(),
		.pBindings = LayoutBindings.data()
	};

	VkResult res = vkCreateDescriptorSetLayout(m_device, &LayoutInfo, nullptr, &m_descriptorSetLayout);
	CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout\n");

	VkPipelineLayoutCreateInfo PipelineLayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &m_descriptorSetLayout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = NULL
	};

	res = vkCreatePipelineLayout(m_device, &PipelineLayoutInfo, nullptr, &m_pipelineLayout);
	CHECK_VK_RESULT(res, "vkCreatePipelineLayout\n");

	VkPipelineShaderStageCreateInfo ShaderStageCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = cs,
		.pName = "main",
		.pSpecializationInfo = NULL
	};

	const VkComputePipelineCreateInfo ci = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.flags = 0,
		.stage = ShaderStageCreateInfo,
		.layout = m_pipelineLayout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};

	res = vkCreateComputePipelines(m_device, NULL, 1, &ci, NULL, &m_pipeline);
	CHECK_VK_RESULT(res, "vkCreateComputePipelines\n");

	AllocateDescriptorSets();
}


ComputePipeline::~ComputePipeline()
{
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, NULL);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, NULL);
	vkDestroyDescriptorPool(m_device, m_descriptorPool, NULL);
	vkDestroyPipeline(m_device, m_pipeline, NULL);
}


void ComputePipeline::RecordCommandBuffer(int Image, VkCommandBuffer CmdBuf, 
										  u32 GroupCountX, u32 GroupCountY, u32 GroupCountZ)
{
	vkCmdBindPipeline(CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
	vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE,
							m_pipelineLayout, 0, 1, &m_descriptorSets[Image], 0, NULL);

	vkCmdDispatch(CmdBuf, GroupCountX, GroupCountY, GroupCountZ);
}


void ComputePipeline::CreateDescriptorPool(u32 TextureCount,
										   u32 UniformBufferCount, 
										   u32 StorageBufferCount, 
										   u32 MaxSets)
{
	// Pool sizes: each entry specifies how many descriptors of that type the pool can allocate.
	std::vector<VkDescriptorPoolSize> PoolSizes;

	if (TextureCount > 0) {
		VkDescriptorPoolSize TexSize = {
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = TextureCount
		};

		PoolSizes.push_back(TexSize);
	}

	if (UniformBufferCount > 0) {
		VkDescriptorPoolSize UboSize = {
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = UniformBufferCount
		};

		PoolSizes.push_back(UboSize);
	}

	if (StorageBufferCount > 0) {
		VkDescriptorPoolSize SsboSize = {
			.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = StorageBufferCount
		};

		PoolSizes.push_back(SsboSize);
	}

	// Optionally include VK_DESCRIPTOR_TYPE_SAMPLER or VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
	// if you use separate sampler/image bindings instead of combined descriptors.

	VkDescriptorPoolCreateInfo PoolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0, 
		.maxSets = MaxSets,
		.poolSizeCount = (u32)PoolSizes.size(),
		.pPoolSizes = PoolSizes.empty() ? NULL : PoolSizes.data()
	};

	VkResult res = vkCreateDescriptorPool(m_device, &PoolCreateInfo, NULL, &m_descriptorPool);
	CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout");
}


void ComputePipeline::AllocateDescriptorSets()
{
	u32 TextureCount = 1;
	u32 UniformBufferCount = 3;
	u32 StorageBufferCount = 0;	// IB, VB, MetaData
	u32 MaxSets = m_numImages;

	m_descriptorSets.resize(m_numImages);

	CreateDescriptorPool(TextureCount, UniformBufferCount, StorageBufferCount, MaxSets);

	AllocateDescriptorSetsInternal(m_descriptorSets);
}


void ComputePipeline::AllocateDescriptorSetsInternal(std::vector<VkDescriptorSet>& DescriptorSets)
{
	std::vector<VkDescriptorSetLayout> Layouts(m_numImages, m_descriptorSetLayout);

	VkDescriptorSetAllocateInfo AllocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = (u32)Layouts.size(),
		.pSetLayouts = Layouts.data()
	};

	DescriptorSets.resize(m_numImages);

    VkResult res = vkAllocateDescriptorSets(m_device, &AllocInfo, DescriptorSets.data());
	CHECK_VK_RESULT(res, "vkAllocateDescriptorSets");	
}


void ComputePipeline::UpdateDescriptorSets(const VulkanTexture& Texture, const std::vector<BufferAndMemory>& UBOs)
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
		VkDescriptorSet& DstSet = m_descriptorSets[ImageIndex];

		VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = 2,
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
			.dstBinding = 3,
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