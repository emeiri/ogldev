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

enum LIGHTING_MODE {
	LIGHTING_MODE_UNLIT        = 0,
	LIGHTING_MODE_NORMALS      = 1,
	LIGHTING_MODE_AMBIENT_ONLY = 2,
	LIGHTING_MODE_FULL         = 3,
	NUM_LIGHTING_MODES         = 4
};


class LightingProgram : public GraphicsPipeline {

public:

	struct UniformDataFS {
		glm::vec4 AmbientLight;      // .rgb = color, .w = intensity
		glm::vec4 LightDirection;    // .xyz = direction
		glm::vec4 LightColor;        // .rgb = color, .w = intensity
	};

	struct UniformDataVS {
		glm::mat4 WVP;
		glm::mat4 NormalMatrix;
	};

	LightingProgram() {}

	virtual void Init(VulkanCore& vkCore,
		VkDescriptorPool DescPool,
		VkDescriptorSetLayout TextureDescSetLayout,
		std::vector<VkDescriptorSet>* pTextureDescSets,
		VkShaderModule vs,
		VkShaderModule fs,
		LIGHTING_MODE LightingMode);

	virtual void Destroy();

	void Bind(int ImageIndex, VkCommandBuffer CmdBuf, VkDescriptorSet& DescSet);

    void AllocDescSets(std::vector<VkDescriptorSet>& DescSets);

	void UpdateDescriptorSets(const ModelDesc& ModelDesc,
							  std::vector<VkDescriptorSet>& DescSets,
							  std::vector<BufferAndMemory>& UniformBuffersVS,
							  std::vector<BufferAndMemory>& UniformBuffersFS);

	void UpdateUniformBuffers(int ImageIndex,
							  const glm::mat4& WVP,
							  const glm::mat4& World,
							  const std::vector<glm::mat4>& SubmeshTransformations,
							  const glm::vec4& AmbientLight,
							  const glm::vec3& LightDirection,
							  std::vector<BufferAndMemory>& UniformBuffersVS,
							  std::vector<BufferAndMemory>& UniformBuffersFS);
protected:

	int m_numImages = 0;

	virtual std::vector<VkDescriptorSetLayout> CreateDescSetLayout(OgldevVK::VulkanCore& vkCore);

    VkDescriptorSetLayout m_textureDescSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet>* m_pTextureDescSets = NULL;
};

}
