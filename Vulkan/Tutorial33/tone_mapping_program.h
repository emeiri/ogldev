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

#pragma once
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ogldev_types.h"
#include "ogldev_vulkan_core.h"
#include "Int/model_desc.h"
#include "ogldev_vulkan_pipeline_program.h"
#include "postprocess_compute_pipeline.h"	// TODO: remove this

namespace OgldevVK {


class ToneMappingProgram : public GraphicsPipeline {

public:

	ToneMappingProgram() {}

	virtual void Init(VulkanCore& vkCore, VkDescriptorPool DescPool);

	virtual void Destroy();

	void Bind(VkCommandBuffer CmdBuf, VkDescriptorSet& DescSet);

    void AllocDescSets(std::vector<VkDescriptorSet>& DescSets);

	void UpdateDescriptorSets(std::vector<VkDescriptorSet>& DescriptorSets,
							  const std::vector<VkImageView>& SwapChainImageViews,
							  const std::vector<OfflineImage>& OfflineImages);

    void RecordCommandBuffer(VkCommandBuffer CmdBuf);

protected:

	virtual std::vector<VkDescriptorSetLayout> CreateDescSetLayout(OgldevVK::VulkanCore& vkCore);

	virtual std::vector<VkPushConstantRange> GetPushConstantRange();

private:

	int m_numImages = 0;
	VkShaderModule m_vs = VK_NULL_HANDLE;
	VkShaderModule m_fs = VK_NULL_HANDLE;
};

}