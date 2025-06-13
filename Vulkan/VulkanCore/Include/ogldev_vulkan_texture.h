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

#include <string>

#include <vulkan/vulkan.h>


namespace OgldevVK {

class VulkanCore;

class VulkanTexture {
public:
	VulkanTexture() {}

	VulkanTexture(VulkanCore* pVulkanCore) { m_pVulkanCore = pVulkanCore; }

	VkImage m_image = VK_NULL_HANDLE;
	VkDeviceMemory m_mem = VK_NULL_HANDLE;
	VkImageView m_view = VK_NULL_HANDLE;
	VkSampler m_sampler = VK_NULL_HANDLE;

	void Destroy(VkDevice Device);

	void Load(const std::string& Filename, bool IsRGB);

	void Load(unsigned int BufferSize, void* pImageData, bool IsRGB);

private:

	VulkanCore* m_pVulkanCore = NULL;

	int m_imageWidth = 0;
	int m_imageHeight = 0;
	int m_imageBPP = 0;
};

}

typedef OgldevVK::VulkanTexture BaseTexture;
typedef OgldevVK::VulkanTexture Texture;