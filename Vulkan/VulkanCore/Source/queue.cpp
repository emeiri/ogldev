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

#include <vulkan/vulkan.h>

#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_queue.h"
#include "ogldev_vulkan_wrapper.h"

namespace OgldevVK {


void VulkanQueue::Init(VkDevice Device, VkSwapchainKHR SwapChain, u32 QueueFamily, u32 QueueIndex)
{
	m_device = Device;
	m_swapChain = SwapChain;

	vkGetDeviceQueue(Device, QueueFamily, QueueIndex, &m_queue);

	VkResult res = vkGetSwapchainImagesKHR(Device, SwapChain, &m_numImages, NULL);
	CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR");

	CreateSyncObjects();
}


void VulkanQueue::Destroy()
{
	for (VkSemaphore& Sem : m_imageAvailableSemaphores) {
		vkDestroySemaphore(m_device, Sem, NULL);
	}

	for (VkSemaphore& Sem : m_renderFinishedSemaphores) {
		vkDestroySemaphore(m_device, Sem, NULL);
	}	

	for (VkFence& Fence : m_inFlightFences) {
		vkDestroyFence(m_device, Fence, NULL);
	}
}


void VulkanQueue::CreateSyncObjects()
{
	m_imageAvailableSemaphores.resize(m_numImages);
	
	for (VkSemaphore& Sem : m_imageAvailableSemaphores) {
		Sem = CreateSemaphore(m_device);
	}

	m_renderFinishedSemaphores.resize(m_numImages);

	for (VkSemaphore& Sem : m_renderFinishedSemaphores) {
		Sem = CreateSemaphore(m_device);
	}

	m_inFlightFences.resize(m_numImages);
	m_imagesInFlight.resize(m_numImages, VK_NULL_HANDLE);

	VkFenceCreateInfo fenceInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT // Start signaled to avoid stalls
	};	
	
	for (VkFence& Fence : m_inFlightFences) {
		VkResult Res = vkCreateFence(m_device, &fenceInfo, NULL, &Fence);
		CHECK_VK_RESULT(Res, "vkCreateFence");
	}
}


void VulkanQueue::WaitIdle()
{
	vkQueueWaitIdle(m_queue);
}


u32 VulkanQueue::AcquireNextImage()
{
	vkWaitForFences(m_device, 1, &m_inFlightFences[m_frameIndex], VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_inFlightFences[m_frameIndex]);

	u32 ImageIndex = 0;

	VkResult res = vkAcquireNextImageKHR(
		m_device,
		m_swapChain,
		UINT64_MAX,
		m_imageAvailableSemaphores[m_frameIndex],
		VK_NULL_HANDLE,
		&ImageIndex
	);
	CHECK_VK_RESULT(res, "vkAcquireNextImageKHR");

	if ((m_imagesInFlight[ImageIndex] != VK_NULL_HANDLE) && 
		(m_imagesInFlight[ImageIndex] != m_inFlightFences[m_frameIndex])) {
		vkWaitForFences(m_device, 1, &m_imagesInFlight[ImageIndex], VK_TRUE, UINT64_MAX);
	}

	m_imagesInFlight[ImageIndex] = m_inFlightFences[m_frameIndex];
	
	return ImageIndex;
}


void VulkanQueue::SubmitSync(VkCommandBuffer CmbBuf)
{
	VkSubmitInfo SubmitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = VK_NULL_HANDLE,
		.pWaitDstStageMask = VK_NULL_HANDLE,
		.commandBufferCount = 1,
		.pCommandBuffers = &CmbBuf,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = VK_NULL_HANDLE
	};

	VkResult res = vkQueueSubmit(m_queue, 1, &SubmitInfo, NULL);
	CHECK_VK_RESULT(res, "vkQueueSubmit\n");
}


void VulkanQueue::SubmitAsync(VkCommandBuffer CmdBuf)
{
	SubmitAsync(&CmdBuf, 1);
}

void VulkanQueue::SubmitAsync(VkCommandBuffer* pCmdBufs, int NumCmdBufs)
{
	VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_imageAvailableSemaphores[m_frameIndex],
		.pWaitDstStageMask = WaitStages,
		.commandBufferCount = (u32)NumCmdBufs,
		.pCommandBuffers = pCmdBufs,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_renderFinishedSemaphores[m_frameIndex]
	};	

	VkResult res = vkQueueSubmit(m_queue, 1, &submitInfo, m_inFlightFences[m_frameIndex]);
	CHECK_VK_RESULT(res, "vkQueueSubmit");
}


void VulkanQueue::Present(u32 ImageIndex)
{
	VkPresentInfoKHR PresentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_renderFinishedSemaphores[m_frameIndex],
		.swapchainCount = 1,
		.pSwapchains = &m_swapChain,
		.pImageIndices = &ImageIndex,
		.pResults = NULL
	};

	VkResult res = vkQueuePresentKHR(m_queue, &PresentInfo);
	CHECK_VK_RESULT(res, "vkQueuePresentKHR");

	m_frameIndex = (m_frameIndex + 1) % m_numImages;
}

}