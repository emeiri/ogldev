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


class VulkanBaseImage {

public:

    VulkanBaseImage() = default;

    void InitVulkanBaseImage(VkImageLayout Layout, VkFormat Format, bool IsSwapChainImage = false) { 
		m_layout = Layout; 
		m_format = Format; 
        m_isSwapChainImage = IsSwapChainImage;
	}

	virtual void Destroy(VkDevice Device);

	void TransitionLayout(VkCommandBuffer CmdBuf, VkImageLayout NewImageLayout);

	void TransitionLayout(VkCommandBuffer CmdBuf, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout);

    VkImageLayout GetLayout() const { return m_layout; }
    
	VkFormat GetFormat() const { return m_format; }

	VkImage m_image = VK_NULL_HANDLE;
	VkImageView m_view = VK_NULL_HANDLE;

protected:
	VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkFormat m_format = VK_FORMAT_UNDEFINED;

private:

    bool m_isSwapChainImage = false;
};

class VulkanTexture : public VulkanBaseImage {

public:
	VulkanTexture() {}

	VulkanTexture(VulkanCore* pVulkanCore) { m_pVulkanCore = pVulkanCore; }

	void Init(VulkanCore* pVulkanCore) { m_pVulkanCore = pVulkanCore; }

	virtual void Destroy(VkDevice Device);

	void Load(const std::string& Filename, bool IsRGB);

	void Load(unsigned int BufferSize, void* pImageData, bool IsRGB);

	void LoadEctCubemap(const std::string& Filename, bool IsRGB);

	void ImageMemoryBarrier(VkCommandBuffer commandBuffer,
							VkImageLayout newImageLayout,
							VkPipelineStageFlags srcStageMask,
							VkPipelineStageFlags dstStageMask);

	VkDeviceMemory m_mem = VK_NULL_HANDLE;
	VkSampler m_sampler = VK_NULL_HANDLE;

private:

	VulkanCore* m_pVulkanCore = NULL;
};


struct OffscreenImage {
	VulkanTexture m_color;
	VulkanTexture m_depth;

	void Destroy(VkDevice Device) {
		m_color.Destroy(Device);
		m_depth.Destroy(Device);
	}
};


}

typedef OgldevVK::VulkanTexture BaseTexture;
typedef OgldevVK::VulkanTexture Texture;