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

enum LIGHTING_MODE {
	LIGHTING_MODE_UNLIT        = 0,
	LIGHTING_MODE_NORMALS      = 1,
	LIGHTING_MODE_AMBIENT_ONLY = 2,
	LIGHTING_MODE_FULL         = 3,
	NUM_LIGHTING_MODES         = 4
};

#define LIGHT_TYPE_DIR   0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT  2

struct LightSource {
	glm::vec3 Color = glm::vec3(0.0f);          // offset 0
	int LightType = 0;							// offset 12
	glm::vec3 Direction = glm::vec3(0.0f);      // offset 16
	float AmbientIntensity = 0.0f;				// offset 28
	glm::vec3 WorldPos = glm::vec3(0.0f);       // offset 32
	float DiffuseIntensity = 0.0f;				// offset 44
	float Atten_Constant = 0.0f;				// offset 48
	float Atten_Linear = 0.0f;					// offset 52
	float Atten_Exp = 0.0f;						// offset 56
	float Cutoff = 0.0f;						// offset 60
};


class LightingProgram : public GraphicsPipeline {

public:

	LightingProgram() {}

	virtual void Init(VulkanCore& vkCore,
					  VkFormat ColorFormat,
					  VkDescriptorPool DescPool,
					  VkDescriptorSetLayout TextureDescSetLayout,
					  VkDescriptorSet TextureDescSet,
					  VkShaderModule vs,
					  VkShaderModule fs,
					  LIGHTING_MODE LightingMode);
	
	virtual void Destroy();

	void Bind(int ImageIndex, VkCommandBuffer CmdBuf, VkDescriptorSet& DescSet, u32 BaseTextureIndex);

    void AllocDescSets(std::vector<VkDescriptorSet>& DescSets);

	void UpdateDescriptorSets(const ModelDesc& ModelDesc,
							  std::vector<VkDescriptorSet>& DescSets,
							  std::vector<BufferAndMemory>& UniformBuffersVS,
							  std::vector<BufferAndMemory>& UniformBuffersFS,
							  std::vector<BufferAndMemory>& UniformBuffersLights,
							  std::vector< std::vector<OgldevVK::VulkanTexture>>& ShadowMaps);

	static void UpdateUniformBuffers(VkDevice Device,
									 const glm::vec3& CameraWorldPos,
									 const glm::mat4& VP,
									 const glm::mat4& World,
									 const PersProjInfo& PersProjInfo,
									 const std::vector<glm::mat4>& SubmeshTransformations,
								     int NumLights,
									 const std::vector<LightSource>& Lights, 
									 BufferAndMemory& UniformBufferVS,
									 BufferAndMemory& UniformBufferFS,
									 BufferAndMemory& UniformBufferLights);

	static size_t GetUniformBufferSizeVS(size_t NumSubmeshes) { return NumSubmeshes * sizeof(UniformDataVS); }

    static size_t GetUniformBufferSizeFS() { return sizeof(UniformDataFS); }

    static size_t GetUniformBufferSizeLights() { return sizeof(UniformBufferLights); }

protected:

	virtual std::vector<VkDescriptorSetLayout> CreateDescSetLayout(OgldevVK::VulkanCore& vkCore);

	virtual std::vector<VkPushConstantRange> GetPushConstantRange();

private:

	struct UniformDataFS {
		glm::ivec4 Counts;              // .x = NumLights, .y = NumPointLights, .z = NumSpotLights, .w = NumDirectionalLights
		glm::vec4 CameraWorldPos;
		glm::vec4 Material;            // .x = specular intensity
		LightSource LightSources[MAX_NUM_LIGHTS];
	};

	struct UniformDataVS {
		glm::mat4 WVP;
        glm::mat4 WorldTransform;
		glm::mat4 NormalTransform;
	};

    struct UniformBufferLights {
        glm::ivec4 Counts;              // .x = NumLights, .y = NumPointLights, .z = NumSpotLights, .w = NumDirectionalLights
        glm::mat4 ViewProj[MAX_NUM_LIGHTS];
    };

	int m_numImages = 0;
    VkDescriptorSetLayout m_textureDescSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet m_textureDescSet = VK_NULL_HANDLE;
	VkSampler m_shadowMapSampler = VK_NULL_HANDLE;
};

}