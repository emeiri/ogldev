/*

		Copyright 2025 Etay Meiri

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

#include <vector>

#include "ogldev_vulkan_texture.h"
#include "ogldev_vulkan_graphics_pipeline_v2.h"

namespace OgldevVK {

	class Skybox {
	public:
		Skybox() {}

		~Skybox() {}

		void Init(VulkanCore* pVulkanCore, const char* pFilename);

		void Destroy();

		void CreateDescriptorSets();

		void RecordCommandBuffer(VkCommandBuffer CmdBuf, int ImageIndex);

		void Update(int ImageIndex, const glm::mat4& Transformation);

	private:

		VulkanCore* m_pVulkanCore = NULL;
		int m_numImages = 0;
		VulkanTexture m_cubemapTex;
		std::vector<BufferAndMemory> m_uniformBuffers;
		std::vector<std::vector<VkDescriptorSet>> m_descriptorSets;
		OgldevVK::GraphicsPipelineV2* m_pPipeline = NULL;
		VkShaderModule m_vs = VK_NULL_HANDLE;
		VkShaderModule m_fs = VK_NULL_HANDLE;
	};
}