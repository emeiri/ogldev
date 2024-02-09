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

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "ogldev_math_3d.h"

#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_device.h"

namespace OgldevVK {


class BufferAndMemory {
public:
	BufferAndMemory() {}
	BufferAndMemory(VkDevice* pDevice) : m_pDevice(pDevice) {}

	VkBuffer m_buffer = NULL;
	VkDeviceMemory m_mem = NULL;
	VkDeviceSize m_allocationSize = 0;

	void Update(const void* pData, size_t Size);

private:
	VkDevice* m_pDevice = NULL;
};


struct TextureAndMemory {
	VkImage m_image = NULL;
	VkDeviceMemory m_mem = NULL;
};

class VulkanCore {
public:
	VulkanCore(GLFWwindow* pWindow);
	~VulkanCore();

	void Init(const char* pAppName, int NumUniformBuffers, size_t UniformDataSize);

	VkDevice& GetDevice() { return m_device; }

	int GetNumImages() const { return (int)m_images.size(); }

	std::vector<VkImage>& GetImages() { return m_images;  }

	uint32_t AcquireNextImage(VkSemaphore Semaphore);

	void Submit(const VkCommandBuffer* pCmbBuf, VkSemaphore PresentCompleteSem, VkSemaphore RenderCompleteSem);

	void QueuePresent(uint32_t ImageIndex, VkSemaphore RenderCompleteSem);

	VkSemaphore CreateSemaphore();

	const VkSurfaceFormatKHR& GetSurfaceFormat() const;

	const VkRenderPass& GetRenderPass() const { return m_renderPass; }

	const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_fbs; }

	VkBuffer CreateVertexBuffer(const std::vector<Vector3f>& Vertices);

	BufferAndMemory CreateUniformBuffer(int Size);

	void CreateTextureImage(const char* filename, TextureAndMemory& Tex);

	void UpdateUniformBuffer(int ImageIndex, int UniformBufferIndex, const void* pData, size_t Size);

	void CreateCommandBuffers(int count, VkCommandBuffer* cmdBufs);

	VkPipeline CreatePipeline(VkShaderModule vs, VkShaderModule fs);

	VkPipelineLayout& GetPipelineLayout() { return m_pipelineLayout; }

	VkDescriptorSet& GetDescriptorSet(int i) { return m_descriptorSets[i]; }

private:

	void CreateInstance(const char* pAppName);
	void CreateDebugCallback();
	void CreateSurface();
	void CreateDevice();
	void CreateSwapChain();
	void CreateRenderPass();
	void CreateFramebuffer();
	void CreateCommandBufferPool();
	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSet();

	uint32_t GetMemoryTypeIndex(uint32_t memTypeBits, VkMemoryPropertyFlags memPropFlags);

	void CopyBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size);

	VkDeviceSize CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties,
					          VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void CreateTextureImageFromData(TextureAndMemory& Tex, const void* pPixels, uint32_t ImageWidth, uint32_t ImageHeight,
									VkFormat TexFormat, uint32_t LayerCount, VkImageCreateFlags Flags);

	void CreateImage(TextureAndMemory& Tex, uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TexFormat, VkImageTiling ImageTiling, 
		             VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags, VkImageCreateFlags CreateFlags, uint32_t MipLevels);

	void UpdateTextureImage(TextureAndMemory& Tex, uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TexFormat, uint32_t LayerCount, const void* pPixels, VkImageLayout SourceImageLayout);

	VkInstance m_instance = NULL;
	VkDebugUtilsMessengerEXT m_messenger;
	GLFWwindow* m_pWindow = NULL;
	VkSurfaceKHR m_surface;
	VulkanPhysicalDevices m_physDevices;
	DeviceAndQueue m_devAndQueue;
	VkDevice m_device;
	VkQueue m_queue;
	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_images;	
	std::vector<VkImageView> m_imageViews;
	VkRenderPass m_renderPass;
	std::vector<VkFramebuffer> m_fbs;
	VkCommandPool m_cmdBufPool;
	VkCommandBuffer m_copyCmdBuf;
	VkPipelineLayout m_pipelineLayout;
	int m_numUniformBuffers = 0;
	size_t m_uniformDataSize = 0;
	std::vector< std::vector<BufferAndMemory> > m_uniformBuffers;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
	std::vector<VkDescriptorSet> m_descriptorSets;
};
}