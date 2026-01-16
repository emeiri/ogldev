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
#include "ogldev_vulkan_shader.h"

namespace OgldevVK {

void ComputePipeline::Init(VulkanCore& vkCore, VkDescriptorPool DescPool, const char* pCSFilename)
{
	m_device = vkCore.GetDevice();
	m_numImages = vkCore.GetNumImages();
	m_descriptorPool = DescPool;

	m_descriptorSetLayout = CreateDescSetLayout(vkCore);

	CreatePipelineLayout();

	m_cs = CreateShaderModuleFromText(m_device, pCSFilename);

	CreatePipeline(m_cs);
}


void ComputePipeline::Destroy()
{
	vkDestroyShaderModule(m_device, m_cs, NULL);
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, NULL);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, NULL);
	vkDestroyPipeline(m_device, m_pipeline, NULL);
}


void ComputePipeline::CreatePipeline(VkShaderModule cs)
{
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

	VkResult res = vkCreateComputePipelines(m_device, NULL, 1, &ci, NULL, &m_pipeline);
	CHECK_VK_RESULT(res, "vkCreateComputePipelines\n");
}


void ComputePipeline::CreatePipelineLayout()
{
	VkPipelineLayoutCreateInfo PipelineLayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &m_descriptorSetLayout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = NULL
	};

	VkResult res = vkCreatePipelineLayout(m_device, &PipelineLayoutInfo, 
		                                  NULL, &m_pipelineLayout);
	CHECK_VK_RESULT(res, "vkCreatePipelineLayout\n");
}


void ComputePipeline::RecordCommandBuffer(VkDescriptorSet DescSet, VkCommandBuffer CmdBuf,
										  u32 GroupCountX, u32 GroupCountY, u32 GroupCountZ)
{
	vkCmdBindPipeline(CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);

	vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE,
							m_pipelineLayout, 0, 1, &DescSet, 0, NULL);

	vkCmdDispatch(CmdBuf, GroupCountX, GroupCountY, GroupCountZ);
}


void ComputePipeline::AllocDescSets(int DescCount, std::vector<VkDescriptorSet>& DescriptorSets)
{
	assert(DescriptorSets.size() == 0);

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


}