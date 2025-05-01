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

#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_graphics_pipeline.h"

namespace OgldevVK {

class VulkanRenderer
{
public:
	VulkanRenderer() {}

	virtual ~VulkanRenderer();
	virtual void FillCommandBuffer(VkCommandBuffer commandBuffer, int Image) = 0;

	virtual void Init(VulkanCore* pvkCore);

	//inline VulkanImage getDepthTexture() const { return depthTexture_; }

protected:

	void BeginRenderPass(VkCommandBuffer commandBuffer, int Image);
	//bool CreateUniformBuffers(int UniformDataSize);

	VulkanCore* m_pvkCore = NULL;
	VkDevice m_device = NULL;

	int m_framebufferWidth  = 0;
	int m_framebufferHeight = 0;

	VkRenderPass m_renderPass = NULL;
	std::vector<VkFramebuffer> m_frameBuffers;
	GraphicsPipeline* m_pPipeline = NULL;
	std::vector<OgldevVK::BufferAndMemory> m_uniformBuffers;
};

}