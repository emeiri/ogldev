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

#include "ogldev_vulkan_core.h"

namespace OgldevVK {

class PipelineProgram {
public:
	PipelineProgram() {}

	void Init(VulkanCore& vkCore, VkDescriptorPool DescPool, const char* pVSFilename, const char* pFSFilename);

	void Destroy();

	void Bind(VkCommandBuffer CmdBuf, VkDescriptorSet DescSet);

	void AllocDescSets(int DescCount, std::vector<VkDescriptorSet>& DescriptorSets);

protected:

	virtual VkDescriptorSetLayout CreateDescSetLayout(OgldevVK::VulkanCore& vkCore) = 0;

	// Default but deriving class may override
	virtual VkCullModeFlags GetCullMode()
	{
		return VK_CULL_MODE_BACK_BIT;
	}

	VkDevice m_device = VK_NULL_HANDLE;

private:

	VkPipeline CreatePipeline(GLFWwindow* pWindow, VkShaderModule vs, VkShaderModule fs,
		VkFormat ColorFormat, VkFormat DepthFormat, VkCompareOp DepthCompareOp);

	VkDescriptorSetLayout m_descSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_descPool = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkShaderModule m_vs = VK_NULL_HANDLE;
	VkShaderModule m_fs = VK_NULL_HANDLE;
};

}