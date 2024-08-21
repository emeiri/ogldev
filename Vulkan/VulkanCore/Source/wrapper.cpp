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

#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_wrapper.h"

namespace OgldevVK {

void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags UsageFlags)
{
	VkCommandBufferBeginInfo BeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = UsageFlags,
		.pInheritanceInfo = NULL
	};

	VkResult res = vkBeginCommandBuffer(CommandBuffer, &BeginInfo);
	CHECK_VK_RESULT(res, "vkBeginCommandBuffer\n");
}


VkSemaphore CreateSemaphore(VkDevice Device)
{
	VkSemaphoreCreateInfo CreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0
	};

	VkSemaphore Semaphore;
	VkResult Res = vkCreateSemaphore(Device, &CreateInfo, NULL, &Semaphore);
	CHECK_VK_RESULT(Res, "vkCreateSemaphore");
	return Semaphore;
}

}