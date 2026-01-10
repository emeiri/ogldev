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

#include <string>
#include <assert.h>

#include <vulkan/vulkan.h>

#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_texture.h"
#include "3rdparty/stb_image.h"
#include "3rdparty/stb_image_write.h"


namespace OgldevVK {

void VulkanTexture::Load(unsigned int BufferSize, void* pData, bool IsRGB)
{
	assert(m_pVulkanCore);

	int Width = 0;
	int Height = 0;
	int BPP = 0;

	void* pImageData = stbi_load_from_memory((const stbi_uc*)pData, BufferSize, &Width, &Height, &BPP, 0);

	m_pVulkanCore->Create2DTextureFromData(pImageData, Width, Height, *this);
}


void VulkanTexture::Load(const std::string& Filename, bool IsRGB)
{
	assert(m_pVulkanCore);

	m_pVulkanCore->CreateTexture(Filename.c_str(), *this);
}


void VulkanTexture::LoadEctCubemap(const std::string& Filename, bool IsRGB)
{
	assert(m_pVulkanCore);
	m_pVulkanCore->CreateCubemapTexture(Filename.c_str(), *this);
}


void VulkanTexture::ImageMemoryBarrier(
	VkCommandBuffer cmd,
	VkImageLayout newLayout,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask)
{
	VkAccessFlags srcAccessMask = 0;
	VkAccessFlags dstAccessMask = 0;

	// ------------------------------------------------------------
	// 1. Derive srcAccessMask from srcStageMask
	// ------------------------------------------------------------

	// Any shader stage may read or write storage images
	if (srcStageMask & (VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
		VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
		VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR))
	{
		srcAccessMask |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	}

	if (srcStageMask & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
		srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	if (srcStageMask & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
		srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	if (srcStageMask & VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)
		srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	if (srcStageMask & VK_PIPELINE_STAGE_TRANSFER_BIT)
		srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

	// ------------------------------------------------------------
	// 2. Derive dstAccessMask from dstStageMask
	// ------------------------------------------------------------

	if (dstStageMask & (VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
		VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
		VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
		VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR))
	{
		dstAccessMask |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	}

	if (dstStageMask & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
		dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	if (dstStageMask & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
		dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	if (dstStageMask & VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)
		dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	if (dstStageMask & VK_PIPELINE_STAGE_TRANSFER_BIT)
		dstAccessMask |= VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

	// ------------------------------------------------------------
	// 3. Build the barrier
	// ------------------------------------------------------------

	VkImageSubresourceRange Range = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = VK_REMAINING_MIP_LEVELS,
		.baseArrayLayer = 0,
		.layerCount = VK_REMAINING_ARRAY_LAYERS
	};

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = m_layout;
	barrier.newLayout = newLayout;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_image;
	barrier.subresourceRange = Range;

	// ------------------------------------------------------------
	// 4. Issue the barrier
	// ------------------------------------------------------------

	vkCmdPipelineBarrier(
		cmd,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	// Track new layout
	m_layout = newLayout;
}


}