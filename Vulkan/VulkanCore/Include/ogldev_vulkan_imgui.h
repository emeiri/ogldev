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

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>

#include "ogldev_vulkan_core.h"

namespace OgldevVK {

class ImGUIRenderer  {
public:
	ImGUIRenderer() {}

	~ImGUIRenderer() {}

	void Init(VulkanCore* pvkCore);

	void Destroy();

	// Called once in every frame where the GUI is visible. Returns
	// a command buffer to be submitted into the queue.
	VkCommandBuffer PrepareCommandBuffer(int Image);

private:

	void CreateDescriptorPool();

	void InitImGUI();	

	VulkanCore* m_pvkCore = NULL;
	int m_framebufferWidth = 0;
	int m_framebufferHeight = 0;
	std::vector<VkCommandBuffer> m_cmdBufs;
	VkDescriptorPool m_descriptorPool = NULL;
};

}

bool IsMouseControlledByImGUI();