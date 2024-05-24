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

#include <stdio.h>

#include <vulkan/vulkan.h>

#include "ogldev_types.h"

namespace OgldevVK {

class VulkanQueue {

public:
	VulkanQueue() {}
	~VulkanQueue() {}

	void Init(VkDevice Device, VkSwapchainKHR SwapChain, u32 QueueFamily, u32 QueueIndex);

	void Destroy();

	u32 AcquireNextImage();

	void SubmitSync(VkCommandBuffer CmbBuf);

	void SubmitAsync(VkCommandBuffer CmbBuf);

	void Present(u32 ImageIndex);

	void WaitIdle();

private:

	void CreateSemaphores();

	void SubmitInternal(VkCommandBuffer CmbBuf, bool IsAsync);

	VkDevice m_device = VK_NULL_HANDLE;
	VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
	VkQueue m_queue = VK_NULL_HANDLE;
	VkSemaphore m_renderCompleteSem;
	VkSemaphore m_presentCompleteSem;
};

}
