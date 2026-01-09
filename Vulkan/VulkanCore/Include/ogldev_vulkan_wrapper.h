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
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "ogldev_types.h"

namespace OgldevVK {

void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags UsageFlags);

VkSemaphore CreateSemaphore(VkDevice Device);

void ImageMemBarrier(VkCommandBuffer CmdBuf, VkImage Image, VkFormat Format,
					 VkImageLayout OldLayout, VkImageLayout NewLayout, int LayerCount);

void BufferMemBarrier(VkCommandBuffer CmdBuf, VkBuffer Buffer, VkPipelineStageFlags SrcStage, VkPipelineStageFlags DstStage);

VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format,
						    VkImageAspectFlags AspectFlags, bool IsCubemap);

VkSampler CreateTextureSampler(VkDevice Device, VkFilter MinFilter, VkFilter MaxFilter,
							   VkSamplerAddressMode AddressMode);
}
