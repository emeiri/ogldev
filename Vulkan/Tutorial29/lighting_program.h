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

struct UniformDataVS {
	glm::mat4 WVP;
	glm::mat4 NormalMatrix;
};

#define MAX_NUM_MESHES (64 * 1024 / sizeof(UniformDataVS))

namespace OgldevVK {

enum LIGHTING_MODE {
	LIGHTING_MODE_UNLIT        = 0,
	LIGHTING_MODE_NORMALS      = 1,
	LIGHTING_MODE_AMBIENT_ONLY = 2,
	LIGHTING_MODE_FULL         = 3,
	NUM_LIGHTING_MODES         = 4
};


struct UniformDataFS {
	glm::vec4 AmbientLight;      // .rgb = color, .w = intensity
	glm::vec4 LightDirection;    // .xyz = direction
	glm::vec4 LightColor;        // .rgb = color, .w = intensity
};


// This version support descriptor indexing and indirect rendering
class GraphicsPipelineV5 : public GraphicsPipeline {

public:

	GraphicsPipelineV5() {}

	virtual void Init(VulkanCore& vkCore,
		VkDescriptorPool DescPool,
		VkShaderModule vs,
		VkShaderModule fs,
		LIGHTING_MODE LightingMode);

	virtual void Destroy();

	void Bind(int ImageIndex, VkCommandBuffer CmdBuf);

	void UpdateDescriptorSets(const ModelDesc& ModelDesc,
							  std::vector<BufferAndMemory>& UniformBuffersVS,
							  std::vector<BufferAndMemory>& UniformBuffersFS);

	void UpdateUniformBuffers(int ImageIndex,
							  const glm::mat4& WVP,
							  const glm::mat4& World,
							  const std::vector<glm::mat4>& SubmeshTransformations,
							  const glm::vec4& AmbientLight,
							  std::vector<BufferAndMemory>& UniformBuffersVS,
							  std::vector<BufferAndMemory>& UniformBuffersFS);
protected:

	VkDescriptorSetLayout CreateDescSetLayout(OgldevVK::VulkanCore& vkCore);

	std::vector<VkDescriptorSet> m_descSets;
};

}
