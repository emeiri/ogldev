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

#include "ogldev_vulkan_pipeline_program.h"

namespace OgldevVK {


class FullScreenQuadProgram : public PipelineProgram {

public:

	FullScreenQuadProgram() {}

	void UpdateDescSets(std::vector<VkDescriptorSet>& DescriptorSets, const OgldevVK::VulkanTexture& Tex);

	void RecordCommandBuffer(VkCommandBuffer CmdBuf);

protected:

	virtual VkDescriptorSetLayout CreateDescSetLayout(OgldevVK::VulkanCore& vkCore);

	virtual VkCullModeFlags GetCullMode() const
	{
		return VK_CULL_MODE_FRONT_BIT;
	}

	u32 SamplerBinding = 0;
};

}