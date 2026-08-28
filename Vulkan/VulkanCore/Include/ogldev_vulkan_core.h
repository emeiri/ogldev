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

#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_device.h"
#include "ogldev_vulkan_queue.h"
#include "ogldev_vulkan_texture.h"

namespace OgldevVK {


enum InitFlags {
    OGLDEV_VK_INIT_NONE = 0x0,
    OGLDEV_VK_INIT_DEPTH_ENABLED = 0x1,
    OGLDEV_VK_INIT_COMPUTE_ENABLED = 0x2,
    OGLDEV_VK_INIT_DISABLE_SWAPCHAIN_SRGB = 0x4
};


class BufferAndMemory {
public:
	BufferAndMemory() {}

	VkBuffer m_buffer = NULL;
	VkDeviceMemory m_mem = NULL;
	VkDeviceSize m_allocationSize = 0;
	VkDeviceAddress m_devAddr = 0;

	void Update(VkDevice Device, const void* pData, size_t Size);

	void Destroy(VkDevice Device);
};


class VulkanCore {

public:

	VulkanCore();

	~VulkanCore();

	void Init(const char* pAppName, GLFWwindow* pWindow, InitFlags Flags);

	VkRenderPass CreateSimpleRenderPass();

	std::vector<VkFramebuffer> CreateFramebuffers(VkRenderPass RenderPass) const;

	void DestroyFramebuffers(std::vector<VkFramebuffer>& Framebuffers);

	VkDevice& GetDevice() { return m_device; }

	const PhysicalDevice& GetPhysicalDevice() { return m_physDevices.Selected(); }

	int GetNumImages() const { return (int)m_images.size(); }

	const VkImage& GetImage(int Index) const;

	const VkImageView& GetImageView(int Index) const;

	const std::vector<VkImageView>& GetImageViews() const { return m_imageViews; }

	const VkImageView& GetDepthView(int Index) const;

	VulkanQueue* GetQueue() { return &m_queue; }

	u32 GetQueueFamily() const { return m_queueFamily; }

	void CreateCommandBuffers(u32 Count, VkCommandBuffer* pCmdBufs);

	void FreeCommandBuffers(u32 Count, const VkCommandBuffer* pCmdBufs);

	BufferAndMemory CreateVertexBuffer(const void* pVertices, size_t Size);

	BufferAndMemory CreateSSBO(const void* pData, size_t Size);

	BufferAndMemory CreateSSBO(size_t Size);

	BufferAndMemory CreateIndirectBuffer(const void* pVertices, size_t Size);

	std::vector<BufferAndMemory> CreateUniformBuffers(size_t Size);

	void CreateTexture(const char* pFilename, VulkanTexture& Tex);

	void CreateTexture(VulkanTexture& Tex, int Width, int Height, VkImageUsageFlags Usage, VkFormat Format, bool EnableAnisotropy);

	void Create2DTextureFromData(const void* pPixels, int ImageWidth, int ImageHeight, VulkanTexture& Tex);

	void CreateCubemapTexture(const char* pFilename, VulkanTexture& Tex);

	void BeginDynamicRenderingSwapChain(VkCommandBuffer CmdBuf, int ImageIndex,
										VkClearValue* pClearColor, VkClearValue* pDepthValue);

    void BeginDynamicRendering(VkCommandBuffer CmdBuf, VkImageView ImageView, VkClearValue* pClearColor, 
							   VkImageView DepthView, VkClearValue* pDepthValue);

	VkFormat GetSwapChainFormat() const { return m_swapChainSurfaceFormat.format; }

	VkFormat GetDepthFormat() const { return m_physDevices.Selected().m_depthFormat; }

	const VkPhysicalDeviceLimits& GetPhysicalDeviceLimits() const;

	GLFWwindow* GetWindow() const { return m_pWindow; }

	VkInstance GetInstance() const { return m_instance; }

	void GetFramebufferSize(int& Width, int& Height) const;

	u32 GetInstanceVersion() const;

	VkDescriptorSetLayout CreateDescSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& Bindings);

	VkDescriptorPool CreateDescPool(u32 TextureCount, u32 UniformBufferCount, u32 StorageBufferCount, u32 StorageImageCount, u32 MaxSets);

	size_t GetMaxUniformBufferSize() const { return m_physDevices.Selected().m_devProps.limits.maxUniformBufferRange; }

	VkExtent2D GetSwapChainExtent() const;

private:

	void UpdateInstanceVersion();
	void CreateInstance(const char* pAppName);
	void CreateDebugCallback();
	void CreateSurface();
	void CreateDevice();
	void CreateSwapChain(bool DisableSRGB);
	void CreateCommandBufferPool();	
	BufferAndMemory CreateUniformBuffer(size_t Size);
	void CreateDepthResources();
	BufferAndMemory CreateBufferInternal(VkBufferUsageFlags Usage, const void* pVertices, size_t Size, bool EnableMapping);

	u32 GetMemoryTypeIndex(u32 memTypeBits, VkMemoryPropertyFlags memPropFlags, bool bExactMatch = false);

	void CopyBufferToBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size);

	BufferAndMemory CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);

	void CreateTextureFromData(const void* pPixels, int ImageWidth, int ImageHeight, VkFormat Format, 
							   bool IsCubemap, VulkanTexture& Tex);

	u32 CreateImageFromData(VulkanTexture& Tex, const void* pPixels, u32 ImageWidth, u32 ImageHeight, 
							VkFormat TexFormat, bool IsCubemap);
	void CreateImage(VulkanTexture& Tex, u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat, VkImageUsageFlags UsageFlags, 
					VkMemoryPropertyFlagBits PropertyFlags, bool IsCubemap, u32 MipLevels);
	void UpdateTextureImage(VulkanTexture& Tex, u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat, 
							int LayerCount, const void* pPixels, bool IsCubemap, u32 MipLevels);
	void CopyBufferToImage(VkImage Dst, VkBuffer Src, u32 ImageWidth, u32 ImageHeight, VkDeviceSize LayerSize, int NumLayers);
	void TransitionImageLayout(VkImage& Image, VkFormat Format, VkImageLayout OldLayout, 
							   VkImageLayout NewLayout, int NumLayers, u32 MipLevels);
	void GenerateMipmaps(VkImage Image, u32 ImageWidth, u32 ImageHeight, VkFormat Format, int LayerCount, u32 MipLevels);

	void SubmitCopyCommand();

    u32 CalcNumMipLevels(VkFormat TexFormat, u32 ImageWidth, u32 ImageHeight) const;

	VkInstance m_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	GLFWwindow* m_pWindow = NULL;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VulkanPhysicalDevices m_physDevices;
	u32 m_queueFamily = 0;
	VkDevice m_device = VK_NULL_HANDLE;
	VkSurfaceFormatKHR m_swapChainSurfaceFormat = {};
	VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
	std::vector<VulkanTexture> m_depthImages;
	VkCommandPool m_cmdBufPool = VK_NULL_HANDLE;
	VulkanQueue m_queue;
	VkCommandBuffer m_copyCmdBuf = VK_NULL_HANDLE;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	bool m_depthEnabled = false;
	struct {
		int Major = 0;
		int Minor = 0;
		int Patch = 0;
	} m_instanceVersion;
};

}