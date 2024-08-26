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

#include <assert.h>

#include <vulkan/vulkan.h>

#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_queue.h"
#include "ogldev_vulkan_wrapper.h"

namespace OgldevVK {


void VulkanQueue::Init(VkDevice Device, u32 QueueFamily, u32 QueueIndex)
{
	m_device = Device;

	vkGetDeviceQueue(Device, QueueFamily, QueueIndex, &m_queue);

	printf("Queue acquired\n");

	CreateSemaphores();
}


void VulkanQueue::SetSwapChain(VkSwapchainKHR SwapChain)
{
	if (m_swapChain) {
		printf("Swap chain already set\n");
		assert(0);
	}

	m_swapChain = SwapChain;
}


void VulkanQueue::Destroy()
{
	vkDestroySemaphore(m_device, m_presentCompleteSem, NULL);
	vkDestroySemaphore(m_device, m_renderCompleteSem, NULL);
}


void VulkanQueue::CreateSemaphores()
{
	m_presentCompleteSem = CreateSemaphore(m_device);
	m_renderCompleteSem = CreateSemaphore(m_device);
}


void VulkanQueue::WaitIdle()
{
	vkQueueWaitIdle(m_queue);
}


u32 VulkanQueue::AcquireNextImage()
{
	assert(m_swapChain);
	u32 ImageIndex = 0;
	VkResult res = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_presentCompleteSem, NULL, &ImageIndex);
	CHECK_VK_RESULT(res, "vkAcquireNextImageKHR\n");
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


void VulkanQueue::SubmitAsync(VkCommandBuffer CmbBuf)
{
	VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo SubmitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_presentCompleteSem,
		.pWaitDstStageMask = &waitFlags,
		.commandBufferCount = 1,
		.pCommandBuffers = &CmbBuf,
		.signalSemaphoreCount = 1,				
		.pSignalSemaphores = &m_renderCompleteSem		
	};

	VkResult res = vkQueueSubmit(m_queue, 1, &SubmitInfo, NULL);
	CHECK_VK_RESULT(res, "vkQueueSubmit\n");
}


void VulkanQueue::Present(u32 ImageIndex)
{
	assert(m_swapChain);

	VkPresentInfoKHR PresentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_renderCompleteSem,
		.swapchainCount = 1,
		.pSwapchains = &m_swapChain,
		.pImageIndices = &ImageIndex
	};

	VkResult res = vkQueuePresentKHR(m_queue, &PresentInfo);
	CHECK_VK_RESULT(res, "vkQueuePresentKHR\n");

	WaitIdle();	// TODO: looks like a workaround but we're getting error messages without it
}

}