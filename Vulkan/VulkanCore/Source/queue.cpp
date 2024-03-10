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

namespace OgldevVK {


// TODO: move this elsewhere
VkSemaphore CreateSemaphore(VkDevice Device)
{
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore semaphore;
	VkResult Res = vkCreateSemaphore(Device, &createInfo, NULL, &semaphore);
	CHECK_VK_RESULT(Res, "vkCreateSemaphore");
	return semaphore;
}


void VulkanQueue::Destroy()
{
	vkDestroySemaphore(m_device, m_presentCompleteSem, NULL);
	vkDestroySemaphore(m_device, m_renderCompleteSem, NULL);
}


void VulkanQueue::Init(VkDevice Device, VkSwapchainKHR SwapChain, u32 QueueFamily, u32 QueueIndex)
{
	m_device = Device;
	m_swapChain = SwapChain;

	vkGetDeviceQueue(Device, QueueFamily, QueueIndex, &m_queue);

	printf("Queue acquired\n");

	CreateSemaphores();
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
	u32 ImageIndex = 0;
	VkResult res = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_presentCompleteSem, NULL, &ImageIndex);
	CHECK_VK_RESULT(res, "vkAcquireNextImageKHR\n");
	return ImageIndex;
}


void VulkanQueue::Submit(const VkCommandBuffer* pCmbBuf)
{
	SubmitInternal(pCmbBuf, false);
}


void VulkanQueue::SubmitAsync(const VkCommandBuffer* pCmbBuf)
{
	SubmitInternal(pCmbBuf, true);
}


void VulkanQueue::SubmitInternal(const VkCommandBuffer* pCmbBuf, bool IsAsync)
{
	VkSubmitInfo SubmitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = pCmbBuf
	};

	if (IsAsync) {
		SubmitInfo.pWaitSemaphores = &m_presentCompleteSem;
		SubmitInfo.waitSemaphoreCount = 1;
		VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		SubmitInfo.pWaitDstStageMask = &waitFlags;

		SubmitInfo.pSignalSemaphores = &m_renderCompleteSem;
		SubmitInfo.signalSemaphoreCount = 1;
	}

	VkResult res = vkQueueSubmit(m_queue, 1, &SubmitInfo, NULL);
	CHECK_VK_RESULT(res, "vkQueueSubmit\n");
}


void VulkanQueue::Present(u32 ImageIndex)
{
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
}

}