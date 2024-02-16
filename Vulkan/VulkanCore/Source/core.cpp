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

#include <vector>
#include <array>
#include <assert.h>

#include "ogldev_vulkan_core.h"
#include "3rdparty/stb_image.h"

namespace OgldevVK {

VulkanCore::VulkanCore(GLFWwindow* pWindow)
{
	m_pWindow = pWindow;
}


VulkanCore::~VulkanCore()
{
	printf("-------------------------------\n");

	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
	vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
	if (!vkDestroyDebugUtilsMessenger) {
		OGLDEV_ERROR("Cannot find address of vkDestroyDebugUtilsMessenger\n");
		exit(1);
	}
	vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, NULL);

	printf("Debug callback destroyed\n");

	vkDestroyInstance(m_instance, NULL);
	printf("Vulkan instance destroyed\n");
}


void VulkanCore::Init(const char* pAppName, int NumUniformBuffers, size_t UniformDataSize)
{
	m_numUniformBuffers = NumUniformBuffers;
	m_uniformDataSize = UniformDataSize;
	CreateInstance(pAppName);
	CreateDebugCallback();
	CreateSurface();
	m_physDevices.Init(m_instance, m_surface);
	m_devAndQueue = m_physDevices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
	CreateDevice();
	CreateSwapChain();
	CreateRenderPass();
	CreateFramebuffer();
	CreateCommandBufferPool();
	CreateCommandBuffers(1, &m_copyCmdBuf);
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSet();
}


void VulkanCore::CreateInstance(const char* pAppName)
{
	std::vector<const char*> Layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	std::vector<const char*> Extensions = {		
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
		"VK_KHR_win32_surface",
#endif
#if defined (__APPLE__)
		"VK_MVK_macos_surface",
#endif
#if defined (__linux__)
		"VK_KHR_xcb_surface",
#endif
	    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};

	VkApplicationInfo AppInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = pAppName,
		.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		.pEngineName = "Ogldev Vulkan Tutorials",
		.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};

	VkInstanceCreateInfo CreateInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pApplicationInfo = &AppInfo,
		.enabledLayerCount = (u32)(Layers.size()),
		.ppEnabledLayerNames = Layers.data(),
		.enabledExtensionCount = (u32)(Extensions.size()),
		.ppEnabledExtensionNames = Extensions.data()
	};

	VkResult res = vkCreateInstance(&CreateInfo, NULL, &m_instance);
	CHECK_VK_RESULT(res, "Create instance");
	printf("Vulkan instance created\n");
}


static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
	VkDebugUtilsMessageTypeFlagsEXT Type,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	printf("Debug callback: %s\n", pCallbackData->pMessage);
	printf("  Severity %s\n", GetDebugSeverityStr(Severity));
	printf("  Type %s\n", GetDebugType(Type));
	printf("  Objects ");

	for (u32 i = 0; i < pCallbackData->objectCount; i++) {
		printf("%llx ", pCallbackData->pObjects[i].objectHandle);
	}

	return VK_FALSE;  // The calling function should not be aborted
}


void VulkanCore::CreateDebugCallback()
{
	VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext = NULL,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
							VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
						VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
						VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = &DebugCallback,
		.pUserData = NULL
	};

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;
	vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
	if (!vkCreateDebugUtilsMessenger) {
		OGLDEV_ERROR("Cannot find address of vkCreateDebugUtilsMessenger\n");
		exit(1);
	}

	VkResult res = vkCreateDebugUtilsMessenger(m_instance, &MessengerCreateInfo, NULL, &m_debugMessenger);
	CHECK_VK_RESULT(res, "debug utils messenger");

	printf("Debug utils messenger created\n");
}


void VulkanCore::CreateSurface()
{
	if (glfwCreateWindowSurface(m_instance, m_pWindow, NULL, &m_surface)) {
		fprintf(stderr, "Error creating GLFW window surface\n");
		exit(1);
	}

	printf("GLFW window surface created\n");
}


void VulkanCore::CreateDevice()
{
	VkDeviceQueueCreateInfo qInfo = {};
	qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	float qPriorities = 1.0f;
	qInfo.queueCount = 1;
	qInfo.pQueuePriorities = &qPriorities;
	qInfo.queueFamilyIndex = m_devAndQueue.Queue;

	std::vector<const char*> DevExts = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
	};

	VkDeviceCreateInfo devInfo = {};
	devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devInfo.enabledExtensionCount = (u32)DevExts.size();
	devInfo.ppEnabledExtensionNames = DevExts.data();
	devInfo.queueCreateInfoCount = 1;
	devInfo.pQueueCreateInfos = &qInfo;

	VkResult res = vkCreateDevice(m_physDevices.m_devices[m_devAndQueue.Device], &devInfo, NULL, &m_device);
	CHECK_VK_RESULT(res, "Create device\n");

	printf("Device created\n");

	vkGetDeviceQueue(m_device, m_devAndQueue.Queue, 0, &m_queue);

	printf("Queue acquired\n");
}


static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& PresentModes)
{
	for (int i = 0; i < PresentModes.size(); i++) {
		if (PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return PresentModes[i];
		}
	}

	// Fallback to FIFO which is always supported
	return VK_PRESENT_MODE_FIFO_KHR;
}


static u32 ChooseNumImages(const VkSurfaceCapabilitiesKHR& Capabilities)
{
	u32 RequestedNumImages = Capabilities.minImageCount + 1;

	int FinalNumImages = 0;

	if ((Capabilities.maxImageCount > 0) && (RequestedNumImages > Capabilities.maxImageCount)) {
		FinalNumImages = Capabilities.maxImageCount;
	}
	else {
		FinalNumImages = RequestedNumImages;
	}

	return FinalNumImages;
}


void VulkanCore::CreateSwapChain()
{
	const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_physDevices.m_surfaceCaps[m_devAndQueue.Device];

	assert(SurfaceCaps.currentExtent.width != -1);

	uint NumImages = ChooseNumImages(SurfaceCaps);

	const std::vector<VkPresentModeKHR>& PresentModes = m_physDevices.m_presentModes[m_devAndQueue.Device];
	VkPresentModeKHR PresentMode = ChoosePresentMode(PresentModes);

	VkSwapchainCreateInfoKHR SwapChainCreateInfo = {};

	SwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapChainCreateInfo.surface = m_surface;
	SwapChainCreateInfo.minImageCount = NumImages;
	SwapChainCreateInfo.imageFormat = m_physDevices.m_surfaceFormats[m_devAndQueue.Device][0].format;
	SwapChainCreateInfo.imageColorSpace = m_physDevices.m_surfaceFormats[m_devAndQueue.Device][0].colorSpace;
	SwapChainCreateInfo.imageExtent = SurfaceCaps.currentExtent;
	SwapChainCreateInfo.imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	SwapChainCreateInfo.preTransform = SurfaceCaps.currentTransform;
	SwapChainCreateInfo.imageArrayLayers = 1;
	SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	SwapChainCreateInfo.presentMode = PresentMode;
	SwapChainCreateInfo.clipped = VK_TRUE;
	SwapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapChainCreateInfo.queueFamilyIndexCount = 1;
	SwapChainCreateInfo.pQueueFamilyIndices = &m_devAndQueue.Queue;

	VkResult res = vkCreateSwapchainKHR(m_device, &SwapChainCreateInfo, NULL, &m_swapChain);
	CHECK_VK_RESULT(res, "vkCreateSwapchainKHR\n");

	printf("Swap chain created\n");

	uint NumSwapChainImages = 0;
	res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, NULL);
	CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");
	assert(NumImages == NumSwapChainImages);

	printf("Number of images %d\n", NumSwapChainImages);

	m_images.resize(NumSwapChainImages);
	m_imageViews.resize(NumSwapChainImages);

	res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, &(m_images[0]));
	CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");

	int LayerCount = 1;
	int MipLevels = 1;
	for (uint i = 0; i < NumSwapChainImages; i++) {
		CreateImageView(m_device, m_images[i], VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &m_imageViews[i], VK_IMAGE_VIEW_TYPE_2D, LayerCount, MipLevels);
	}
}


u32 VulkanCore::AcquireNextImage(VkSemaphore Semaphore)
{
	u32 ImageIndex = 0;
	VkResult res = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, Semaphore, NULL, &ImageIndex);
	CHECK_VK_RESULT(res, "vkAcquireNextImageKHR\n");
	return ImageIndex;
}


void VulkanCore::Submit(const VkCommandBuffer* pCmbBuf, VkSemaphore PresentCompleteSem, VkSemaphore RenderCompleteSem)
{
	VkSubmitInfo submitInfo = {};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = pCmbBuf;
	if (PresentCompleteSem) {
		submitInfo.pWaitSemaphores = &PresentCompleteSem;
		submitInfo.waitSemaphoreCount = 1;
		VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.pWaitDstStageMask = &waitFlags;
	}

	if (RenderCompleteSem) {
		submitInfo.pSignalSemaphores = &RenderCompleteSem;
		submitInfo.signalSemaphoreCount = 1;
	}

	VkResult res = vkQueueSubmit(m_queue, 1, &submitInfo, NULL);
	CHECK_VK_RESULT(res, "vkQueueSubmit\n");
}


void VulkanCore::QueuePresent(u32 ImageIndex, VkSemaphore RenderCompleteSem)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapChain;
	presentInfo.pImageIndices = &ImageIndex;
	presentInfo.pWaitSemaphores = &RenderCompleteSem;
	presentInfo.waitSemaphoreCount = 1;

	VkResult res = vkQueuePresentKHR(m_queue, &presentInfo);
	CHECK_VK_RESULT(res, "vkQueuePresentKHR\n");
}


VkSemaphore VulkanCore::CreateSemaphore()
{
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore semaphore;
	VkResult res = vkCreateSemaphore(m_device, &createInfo, NULL, &semaphore);
	CHECK_VK_RESULT(res, "vkCreateSemaphore\n");
	return semaphore;
}


const VkSurfaceFormatKHR& VulkanCore::GetSurfaceFormat() const
{
	return m_physDevices.m_surfaceFormats[m_devAndQueue.Device][0];
}


void VulkanCore::CreateRenderPass()
{
	VkAttachmentReference attachRef = {};
	attachRef.attachment = 0;
	attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &attachRef;

	VkAttachmentDescription attachDesc = {};
	attachDesc.format = GetSurfaceFormat().format;
	attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;

#if 0
	VkAttachmentDescription depthAttachment = {
		.flags = 0,
		.Format = useDepth ? findDepthFormat(vkDev.physicalDevice) : VK_FORMAT_D32_SFLOAT,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = offscreenInt ? VK_ATTACHMENT_LOAD_OP_LOAD : (ci.clearDepth_ ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD),
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = ci.clearDepth_ ? VK_IMAGE_LAYOUT_UNDEFINED : (offscreenInt ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	const VkAttachmentReference depthAttachmentRef = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
#endif

	VkSubpassDependency Dependency = {};
	Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo RenderPassCreateInfo = {};
	RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassCreateInfo.attachmentCount = 1;
	RenderPassCreateInfo.pAttachments = &attachDesc;
	RenderPassCreateInfo.subpassCount = 1;
	RenderPassCreateInfo.pSubpasses = &subpassDesc;
	RenderPassCreateInfo.dependencyCount = 1;
	RenderPassCreateInfo.pDependencies = &Dependency;

	VkResult res = vkCreateRenderPass(m_device, &RenderPassCreateInfo, NULL, &m_renderPass);
	CHECK_VK_RESULT(res, "vkCreateRenderPass\n");

	printf("Created a render pass\n");
}


void VulkanCore::CreateFramebuffer()
{
	m_fbs.resize(m_images.size());

	int WindowWidth, WindowHeight;
	glfwGetWindowSize(m_pWindow, &WindowWidth, &WindowHeight);

	VkResult res;

	for (uint i = 0; i < m_images.size(); i++) {

		VkFramebufferCreateInfo fbCreateInfo = {};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.renderPass = m_renderPass;
		fbCreateInfo.attachmentCount = 1;
		fbCreateInfo.pAttachments = &m_imageViews[i];
		fbCreateInfo.width = WindowWidth;
		fbCreateInfo.height = WindowHeight;
		fbCreateInfo.layers = 1;

		res = vkCreateFramebuffer(m_device, &fbCreateInfo, NULL, &m_fbs[i]);
		CHECK_VK_RESULT(res, "vkCreateFramebuffer\n");
	}

	printf("Framebuffers created\n");
}


VkBuffer VulkanCore::CreateVertexBuffer(const void* pVertices, size_t Size)
{
	VkBuffer StagingVB;
	VkDeviceMemory StagingVBMem;
	VkDeviceSize AllocationSize = CreateBuffer(Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingVB, StagingVBMem);

	void* MappedMemAddr = NULL;
	VkResult res = vkMapMemory(m_device, StagingVBMem, 0, AllocationSize, 0, &MappedMemAddr);
	memcpy(MappedMemAddr, pVertices, Size);
	vkUnmapMemory(m_device, StagingVBMem);

	VkBuffer vb;
	VkDeviceMemory vbMem;
	AllocationSize = CreateBuffer(Size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vb, vbMem);

	CopyBuffer(vb, StagingVB, Size);

	return vb;
}


BufferAndMemory VulkanCore::CreateUniformBuffer(int Size)
{
	BufferAndMemory Buffer(&m_device);

	Buffer.m_allocationSize = CreateBuffer(Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Buffer.m_buffer, Buffer.m_mem);

	return Buffer;
}


VkDeviceSize VulkanCore::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties,
	VkBuffer& Buffer, VkDeviceMemory& BufferMemory)
{
	VkBufferCreateInfo vbCreateInfo = {};
	vbCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vbCreateInfo.size = Size;
	vbCreateInfo.usage = Usage;
	vbCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult res = vkCreateBuffer(m_device, &vbCreateInfo, NULL, &Buffer);
	CHECK_VK_RESULT(res, "vkCreateBuffer\n");
	printf("Create vertex buffer\n");

	VkMemoryRequirements MemReqs = {};
	vkGetBufferMemoryRequirements(m_device, Buffer, &MemReqs);
	printf("Vertex buffer requires %d bytes\n", (int)MemReqs.size);

	u32 MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, Properties);
	VkMemoryAllocateInfo MemAllocInfo = {};
	MemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemAllocInfo.allocationSize = MemReqs.size;
	MemAllocInfo.memoryTypeIndex = MemoryTypeIndex;
	printf("Memory type index %d\n", MemAllocInfo.memoryTypeIndex);

	res = vkAllocateMemory(m_device, &MemAllocInfo, NULL, &BufferMemory);
	CHECK_VK_RESULT(res, "vkAllocateMemory error %d\n");

	res = vkBindBufferMemory(m_device, Buffer, BufferMemory, 0);
	CHECK_VK_RESULT(res, "vkBindBufferMemory error %d\n");

	return MemAllocInfo.allocationSize;
}


void VulkanCore::CreateTextureImage(const char* filename, TextureAndMemory& Tex)
{
	int ImageWidth = 0;
	int ImageHeight = 0;
	int ImageChannels = 0;

	stbi_uc* pPixels = stbi_load(filename, &ImageWidth, &ImageHeight, &ImageChannels, STBI_rgb_alpha);

	if (!pPixels) {
		printf("Error loading texture from '%s'\n", filename);
		exit(1);
	}

	u32 LayerCount = 1;
	VkImageCreateFlags Flags = 0;
	VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;
	CreateTextureImageFromData(Tex, pPixels, ImageWidth, ImageHeight, Format, LayerCount, Flags);

	stbi_image_free(pPixels);

	VkImageViewType ViewType = VK_IMAGE_VIEW_TYPE_2D;
	VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	u32 MipLevels = 1;

	CreateImageView(m_device, Tex.m_image, Format, AspectFlags, &Tex.m_view, ViewType, LayerCount, MipLevels);

	VkFilter MinFilter = VK_FILTER_LINEAR;
	VkFilter MaxFilter = VK_FILTER_LINEAR;
	VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	CreateTextureSampler(m_device, &Tex.m_sampler, MinFilter, MaxFilter, AddressMode);
}


void VulkanCore::CreateTextureImageFromData(TextureAndMemory& Tex, const void* pPixels, u32 ImageWidth, u32 ImageHeight,
											VkFormat TexFormat, u32 LayerCount, VkImageCreateFlags CreateFlags)
{
	CreateImage(Tex, ImageWidth, ImageHeight, TexFormat, VK_IMAGE_TILING_OPTIMAL, 
			    (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, CreateFlags, 1);

	UpdateTextureImage(Tex, ImageWidth, ImageHeight, TexFormat, LayerCount, pPixels, VK_IMAGE_LAYOUT_UNDEFINED);
}


void VulkanCore::CreateImage(TextureAndMemory& Tex, u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat, VkImageTiling ImageTiling,
	                         VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags, VkImageCreateFlags CreateFlags, u32 MipLevels)
{
	VkImageCreateInfo ImageInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = CreateFlags,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = TexFormat,
		.extent = VkExtent3D {.width = ImageWidth, .height = ImageHeight, .depth = 1 },
		.mipLevels = MipLevels,
		.arrayLayers = (u32)((CreateFlags == VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ? 6 : 1),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = ImageTiling,
		.usage = UsageFlags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VkResult res = vkCreateImage(m_device, &ImageInfo, NULL, &Tex.m_image);
	CHECK_VK_RESULT(res, "vkCreateImage error");

	VkMemoryRequirements MemReqs;
	vkGetImageMemoryRequirements(m_device, Tex.m_image, &MemReqs);

	u32 MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, PropertyFlags);

	VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize = MemReqs.size,
		.memoryTypeIndex = MemoryTypeIndex
	};

	res = vkAllocateMemory(m_device, &allocInfo, NULL, &Tex.m_mem);
	CHECK_VK_RESULT(res, "vkAllocateMemory error");

	vkBindImageMemory(m_device, Tex.m_image, Tex.m_mem, 0);
}


void VulkanCore::UpdateTextureImage(TextureAndMemory& Tex, u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat, u32 LayerCount, const void* pPixels, VkImageLayout SourceImageLayout)
{
	u32 BytesPerPixel = GetBytesPerTexFormat(TexFormat);

	VkDeviceSize LayerSize = ImageWidth * ImageHeight * BytesPerPixel;
	VkDeviceSize ImageSize = LayerSize * LayerCount;

	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;

	VkDeviceSize AllocationSize = CreateBuffer(ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);

	UploadBufferData(StagingBufferMemory, 0, pPixels, ImageSize);

	TransitionImageLayout(Tex.m_image, TexFormat, SourceImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, LayerCount, 1);
	CopyBufferToImage(StagingBuffer, Tex.m_image, ImageWidth, ImageHeight, LayerCount);
	TransitionImageLayout(Tex.m_image, TexFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, LayerCount, 1);

	vkDestroyBuffer(m_device, StagingBuffer, NULL);
	vkFreeMemory(m_device, StagingBufferMemory, NULL);
}


void VulkanCore::UploadBufferData(const VkDeviceMemory& BufferMemory, VkDeviceSize DeviceOffset, const void* pData, const size_t DataSize)
{
	void* pMappedData = NULL;

	vkMapMemory(m_device, BufferMemory, DeviceOffset, DataSize, 0, &pMappedData);
	
	memcpy(pMappedData, pData, DataSize);
	
	vkUnmapMemory(m_device, BufferMemory);
}


void VulkanCore::CopyBufferToImage(VkBuffer Buffer, VkImage Image, u32 ImageWidth, u32 ImageHeight, u32 LayerCount)
{
	VkCommandBuffer CmdBuf = BeginSingleUseCommand();

	VkBufferImageCopy Region = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = VkImageSubresourceLayers {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = LayerCount
		},
		.imageOffset = VkOffset3D {.x = 0, .y = 0, .z = 0 },
		.imageExtent = VkExtent3D {.width = ImageWidth, .height = ImageHeight, .depth = 1 }
	};

	vkCmdCopyBufferToImage(CmdBuf, Buffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);

	EndSingleTimeCommands(CmdBuf);
}



void VulkanCore::TransitionImageLayout(VkImage& Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout, u32 LayerCount, u32 MipLevels)
{
	VkCommandBuffer CmdBuf = BeginSingleUseCommand();

	TransitionImageLayoutCmd(CmdBuf, Image, Format, OldLayout, NewLayout, LayerCount, MipLevels);

	EndSingleTimeCommands(CmdBuf);
}


void VulkanCore::TransitionImageLayoutCmd(VkCommandBuffer CmdBuf, VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout, u32 LayerCount, u32 MipLevels)
{
	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = 0,
		.dstAccessMask = 0,
		.oldLayout = OldLayout,
		.newLayout = NewLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = Image,
		.subresourceRange = VkImageSubresourceRange {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = MipLevels,
			.baseArrayLayer = 0,
			.layerCount = LayerCount
		}
	};

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
		(Format == VK_FORMAT_D16_UNORM) ||
		(Format == VK_FORMAT_X8_D24_UNORM_PACK32) ||
		(Format == VK_FORMAT_D32_SFLOAT) ||
		(Format == VK_FORMAT_S8_UINT) ||
		(Format == VK_FORMAT_D16_UNORM_S8_UINT) ||
		(Format == VK_FORMAT_D24_UNORM_S8_UINT))
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(Format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
		if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_GENERAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
	if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	/* Convert back from read-only to updateable */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	/* Convert from updateable texture to shader read-only */
	else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	/* Convert depth texture from undefined state to depth-stencil buffer */
	else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}

	/* Wait for render pass to complete */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = 0;
		/*
				sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		///		destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		*/
		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	/* Convert back from read-only to color attachment */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	/* Convert from updateable texture to shader read-only */
	else if (OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	/* Convert back from read-only to depth attachment */
	else if (OldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}
	/* Convert from updateable depth texture to shader read-only */
	else if (OldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	vkCmdPipelineBarrier(
		CmdBuf,
		sourceStage, destinationStage,
		0,
		0, NULL,
		0, NULL,
		1, &barrier
	);
}



static void BeginOneTimeCommandBuffer(VkCommandBuffer CmdBuf)
{
	VkCommandBufferBeginInfo cmdBufBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = NULL
	};

	VkResult res = vkBeginCommandBuffer(CmdBuf, &cmdBufBeginInfo);
	CHECK_VK_RESULT(res, "vkBeginCommandBuffer error %d\n");
}


void VulkanCore::CopyBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size)
{
	BeginOneTimeCommandBuffer(m_copyCmdBuf);

	VkBufferCopy bufferCopy = {};
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = Size;
	vkCmdCopyBuffer(m_copyCmdBuf, Src, Dst, 1, &bufferCopy);

	vkEndCommandBuffer(m_copyCmdBuf);

	Submit(&m_copyCmdBuf, NULL, NULL);

	vkQueueWaitIdle(m_queue);
}


u32 VulkanCore::GetMemoryTypeIndex(u32 memTypeBits, VkMemoryPropertyFlags reqMemPropFlags)
{
	VkPhysicalDeviceMemoryProperties& MemProps = m_physDevices.m_memProps[m_devAndQueue.Device];

	for (uint i = 0; i < MemProps.memoryTypeCount; i++) {
		if ((memTypeBits & (1 << i)) &&
			((MemProps.memoryTypes[i].propertyFlags & reqMemPropFlags) == reqMemPropFlags)) {
			return i;
		}
	}

	printf("Cannot find memory type for type %x requested mem props %x\n", memTypeBits, reqMemPropFlags);
	exit(1);
	return -1;
}


void VulkanCore::CreateCommandBufferPool()
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.queueFamilyIndex = m_devAndQueue.Queue;

	VkResult res = vkCreateCommandPool(m_device, &cmdPoolCreateInfo, NULL, &m_cmdBufPool);
	CHECK_VK_RESULT(res, "vkCreateCommandPool\n");

	printf("Command buffer pool created\n");
}


void VulkanCore::CreateCommandBuffers(u32 count, VkCommandBuffer* cmdBufs)
{
	VkCommandBufferAllocateInfo cmdBufAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = m_cmdBufPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = count
	};

	VkResult res = vkAllocateCommandBuffers(m_device, &cmdBufAllocInfo, cmdBufs);
	CHECK_VK_RESULT(res, "vkAllocateCommandBuffers\n");

	printf("Created %d command buffers\n", count);
}


VkCommandBuffer VulkanCore::BeginSingleUseCommand()
{
	VkCommandBuffer CmdBuf;

	CreateCommandBuffers(1, &CmdBuf);

	BeginOneTimeCommandBuffer(CmdBuf);

	return CmdBuf;
}


void VulkanCore::EndSingleTimeCommands(VkCommandBuffer CmdBuf)
{
	vkEndCommandBuffer(CmdBuf);

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = nullptr,
		.pWaitDstStageMask = nullptr,
		.commandBufferCount = 1,
		.pCommandBuffers = &CmdBuf,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = nullptr
	};

	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);

	vkFreeCommandBuffers(m_device, m_cmdBufPool, 1, &CmdBuf);
}


VkPipeline VulkanCore::CreatePipeline(VkShaderModule vs, VkShaderModule fs)
{
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};

	shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfo[0].module = vs;
	shaderStageCreateInfo[0].pName = "main";

	shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfo[1].module = fs;
	shaderStageCreateInfo[1].pName = "main";

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo pipelineIACreateInfo = {};
	pipelineIACreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineIACreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineIACreateInfo.primitiveRestartEnable = VK_FALSE;

	int WindowWidth, WindowHeight;
	glfwGetWindowSize(m_pWindow, &WindowWidth, &WindowHeight);

	VkViewport vp = {};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = (float)WindowWidth;
	vp.height = (float)WindowHeight;
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = WindowWidth;
	scissor.extent.height = WindowHeight;

	VkPipelineViewportStateCreateInfo vpCreateInfo = {};
	vpCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vpCreateInfo.viewportCount = 1;
	vpCreateInfo.pViewports = &vp;
	vpCreateInfo.scissorCount = 1;
	vpCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
	rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rastCreateInfo.cullMode = VK_CULL_MODE_NONE;
	rastCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rastCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo pipelineMSCreateInfo = {};
	pipelineMSCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMSCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineMSCreateInfo.sampleShadingEnable = VK_FALSE;
	pipelineMSCreateInfo.minSampleShading = 1.0f;

	VkPipelineColorBlendAttachmentState blendAttachState = {};
	blendAttachState.blendEnable = VK_FALSE;
	blendAttachState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
	blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendCreateInfo.logicOpEnable = VK_FALSE;
	blendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	blendCreateInfo.attachmentCount = 1;
	blendCreateInfo.pAttachments = &blendAttachState;

	VkPipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1;
	layoutInfo.pSetLayouts = &m_descriptorSetLayout;

	VkResult res = vkCreatePipelineLayout(m_device, &layoutInfo, NULL, &m_pipelineLayout);
	CHECK_VK_RESULT(res, "vkCreatePipelineLayout\n");

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = ARRAY_SIZE_IN_ELEMENTS(shaderStageCreateInfo);
	pipelineInfo.pStages = &shaderStageCreateInfo[0];
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &pipelineIACreateInfo;
	pipelineInfo.pViewportState = &vpCreateInfo;
	pipelineInfo.pRasterizationState = &rastCreateInfo;
	pipelineInfo.pMultisampleState = &pipelineMSCreateInfo;
	pipelineInfo.pColorBlendState = &blendCreateInfo;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline Pipeline;
	res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &Pipeline);
	CHECK_VK_RESULT(res, "vkCreateGraphicsPipelines\n");

	printf("Graphics pipeline created\n");

	return Pipeline;
}


void VulkanCore::CreateUniformBuffers()
{
	m_uniformBuffers.resize(m_images.size());

	for (int i = 0; i < m_uniformBuffers.size(); i++) {
		m_uniformBuffers[i].resize(m_numUniformBuffers);

		for (int j = 0; j < m_numUniformBuffers; j++) {
			m_uniformBuffers[i][j] = CreateUniformBuffer((int)m_uniformDataSize);
		}
	}
}


void VulkanCore::CreateDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> PoolSizes;

	VkDescriptorPoolSize DescPoolSize = {};
	DescPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	DescPoolSize.descriptorCount = (u32)m_images.size() * m_numUniformBuffers;

	VkDescriptorPoolCreateInfo PoolInfo = { };

	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.maxSets = (u32)m_images.size();
	PoolInfo.poolSizeCount = 1;
	PoolInfo.pPoolSizes = &DescPoolSize;

	VkResult res = vkCreateDescriptorPool(m_device, &PoolInfo, NULL, &m_descriptorPool);
	CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
	printf("Descriptor pool created\n");
}


void VulkanCore::CreateDescriptorSet()
{
	std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

	VkDescriptorSetLayoutBinding VertexShaderLayoutBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = VK_NULL_HANDLE
	};

	VkDescriptorSetLayoutBinding FragmentShaderLayoutBinding{
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = VK_NULL_HANDLE
	};

	LayoutBindings.push_back(VertexShaderLayoutBinding);
	LayoutBindings.push_back(FragmentShaderLayoutBinding);

	VkDescriptorSetLayoutCreateInfo LayoutInfo = {};

	LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	LayoutInfo.flags = 0;
	LayoutInfo.bindingCount = (u32)LayoutBindings.size();
	LayoutInfo.pBindings = LayoutBindings.data();

	VkResult res = vkCreateDescriptorSetLayout(m_device, &LayoutInfo, NULL, &m_descriptorSetLayout);
	CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout");

	std::vector<VkDescriptorSetLayout> Layouts(m_images.size(), m_descriptorSetLayout);

	VkDescriptorSetAllocateInfo AllocInfo = {};
	AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocInfo.descriptorPool = m_descriptorPool;
	AllocInfo.descriptorSetCount = (u32)(m_images.size());
	AllocInfo.pSetLayouts = Layouts.data();

	m_descriptorSets.resize(m_images.size());

	res = vkAllocateDescriptorSets(m_device, &AllocInfo, m_descriptorSets.data());
	CHECK_VK_RESULT(res, "vkAllocateDescriptorSets");

	for (size_t i = 0; i < m_images.size(); i++) {
		VkDescriptorBufferInfo BufferInfo = {
			.buffer = m_uniformBuffers[i][0].m_buffer,
			.offset = 0,
			.range = m_uniformDataSize,
		};

		VkDescriptorImageInfo ImageInfo = {
			//.sampler =  TODO_EMEIRI
			.imageView = 0,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};

		std::array<VkWriteDescriptorSet, 2> WriteDescriptorSet = {
			VkWriteDescriptorSet {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pBufferInfo = &BufferInfo
			},
			VkWriteDescriptorSet {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_descriptorSets[i],
				.dstBinding = 3,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &ImageInfo
			},
		};

		/*WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		WriteDescriptorSet.dstSet = m_descriptorSets[i];
		WriteDescriptorSet.dstBinding = 0;
		WriteDescriptorSet.dstArrayElement = 0;
		WriteDescriptorSet.descriptorCount = 1;
		WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		WriteDescriptorSet.pBufferInfo = &BufferInfo1;*/

		vkUpdateDescriptorSets(m_device, (u32)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
	}
}


void VulkanCore::UpdateUniformBuffer(int ImageIndex, int UniformBufferIndex, const void* pData, size_t Size)
{
	if (ImageIndex >= m_uniformBuffers.size()) {
		OGLDEV_ERROR("UpdateUniformBuffer: image index %d array size %d\n", ImageIndex, m_uniformBuffers.size());
	}

	if (UniformBufferIndex >= m_numUniformBuffers) {
		OGLDEV_ERROR("UpdateUniformBuffer: uniform buffer index %d num uniform buffers %d\n", UniformBufferIndex, m_numUniformBuffers);
	}

	m_uniformBuffers[ImageIndex][UniformBufferIndex].Update(pData, Size);
}


void BufferAndMemory::Update(const void* pData, size_t Size)
{
	if (!m_pDevice) {
		OGLDEV_ERROR("Buffer has not been initialized with a device pointer");
	}

	void* pMem = NULL;
	vkMapMemory(*m_pDevice, m_mem, 0, Size, 0, &pMem);
	memcpy(pMem, pData, Size);
	vkUnmapMemory(*m_pDevice, m_mem);
}

}
