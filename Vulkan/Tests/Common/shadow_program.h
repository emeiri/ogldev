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

namespace OgldevVK {

#define MAX_NUM_LIGHTS 16

#define LIGHT_TYPE_DIR   0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT  2

class ShadowProgram : public GraphicsPipeline {

public:

	ShadowProgram() {}

	virtual void Init(VulkanCore& vkCore, VkDescriptorPool DescPool,
				      const std::vector<VkDescriptorSetLayout>& LightingDescSetLayouts);

	virtual void Destroy();

	void Bind(int ImageIndex,
			  VkCommandBuffer CmdBuf,
			  VkDescriptorSet DescSet,
			  VkDescriptorSet TextureDescSet,
			  u32 LightIndex);

	static size_t GetUniformBufferSizeVS(size_t NumSubmeshes) { return NumSubmeshes * sizeof(UniformDataVS); }

    static size_t GetUniformBufferSizeFS() { return sizeof(UniformDataFS); }

protected:

	virtual std::vector<VkDescriptorSetLayout> CreateDescSetLayout(VulkanCore& vkCore);

	virtual std::vector<VkPushConstantRange> GetPushConstantRange();

private:

	struct UniformDataFS {
		glm::ivec4 Counts;              // .x = NumLights, .y = NumPointLights, .z = NumSpotLights, .w = NumDirectionalLights
	};

	struct UniformDataVS {
		glm::mat4 WVP;
        glm::mat4 WorldTransform;
	};

	int m_numImages = 0;
	VkShaderModule m_vs;
	VkShaderModule m_fs;
    std::vector<VkDescriptorSetLayout> m_lightingDescSetLayouts;
};

}