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

#include "ogldev_vulkan_compute_pipeline.h"

namespace OgldevVK {

ComputePipeline::ComputePipeline(const ComputePipelineDesc& pd)
{
	m_device = pd.Device;
	m_numImages = pd.NumImages;

//	CreateDescriptorSetLayout(pd.IsVB, pd.IsIB, pd.IsTex2D, pd.IsUniform);

	VkPipelineShaderStageCreateInfo ShaderStageCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = pd.cs,
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

}