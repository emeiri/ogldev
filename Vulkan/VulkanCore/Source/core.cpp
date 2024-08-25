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
#include <assert.h>

#include "ogldev_types.h"
#include "ogldev_util.h"
#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_wrapper.h"

namespace OgldevVK {

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
#ifdef _WIN32
		printf("%llux ", pCallbackData->pObjects[i].objectHandle);
#else
		printf("%lux ", pCallbackData->pObjects[i].objectHandle);
#endif
	}

	printf("\n");

	return VK_FALSE;  // The calling function should not be aborted
}


VulkanCore::VulkanCore()
{
}


VulkanCore::~VulkanCore()
{
	printf("-------------------------------\n");

	vkFreeCommandBuffers(m_device, m_cmdBufPool, 1, &m_copyCmdBuf);

	vkDestroyCommandPool(m_device, m_cmdBufPool, NULL);

	m_queue.Destroy();

	for (int i = 0; i < m_imageViews.size(); i++) {
		vkDestroyImageView(m_device, m_imageViews[i], NULL);
	}

	vkDestroySwapchainKHR(m_device, m_swapChain, NULL);

	vkDestroyDevice(m_device, NULL);

	PFN_vkDestroySurfaceKHR vkDestroySurface = VK_NULL_HANDLE;
	vkDestroySurface = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkDestroySurfaceKHR");
	if (!vkDestroySurface) {
		OGLDEV_ERROR0("Cannot find address of vkDestroySurfaceKHR\n");
		exit(1);
	}

	vkDestroySurface(m_instance, m_surface, NULL);

	printf("GLFW window surface destroyed\n");

	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
	vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
	if (!vkDestroyDebugUtilsMessenger) {
		OGLDEV_ERROR0("Cannot find address of vkDestroyDebugUtilsMessengerEXT\n");
		exit(1);
	}
	vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, NULL);

	printf("Debug callback destroyed\n");

	vkDestroyInstance(m_instance, NULL);
	printf("Vulkan instance destroyed\n");
}


void VulkanCore::Init(const char* pAppName, GLFWwindow* pWindow)
{
	m_pWindow = pWindow;
	CreateInstance(pAppName);
	CreateDebugCallback();
	if (!pWindow) {
		printf("You are probably in one of the initial tutorials so we can end the Init function here.\n");
		return;
	}
	CreateSurface();
	m_physDevices.Init(m_instance, m_surface);
	m_queueFamily = m_physDevices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
	CreateDevice();
	CreateSwapChain();
	CreateCommandBufferPool();
	m_queue.Init(m_device, m_swapChain, m_queueFamily, 0);
	CreateCommandBuffers(1, &m_copyCmdBuf);
}


const VkImage& VulkanCore::GetImage(int Index) const
{
	if (Index >= m_images.size()) {
		OGLDEV_ERROR("Invalid image index %d\n", Index);
		exit(1);
	}

	return m_images[Index];
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
		.pNext = &MessengerCreateInfo,
		.flags = 0,				// reserved for future use. Must be zero
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
		OGLDEV_ERROR0("Cannot find address of vkCreateDebugUtilsMessenger\n");
		exit(1);
	}

	VkResult res = vkCreateDebugUtilsMessenger(m_instance, &MessengerCreateInfo, NULL, &m_debugMessenger);
	CHECK_VK_RESULT(res, "debug utils messenger");

	printf("Debug utils messenger created\n");
}


void VulkanCore::CreateSurface()
{
	VkResult res = glfwCreateWindowSurface(m_instance, m_pWindow, NULL, &m_surface);
	CHECK_VK_RESULT(res, "glfwCreateWindowSurface");

	printf("GLFW window surface created\n");
}


void VulkanCore::CreateDevice()
{
	float qPriorities[] = { 1.0f };

	VkDeviceQueueCreateInfo qInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0, // must be zero
		.queueFamilyIndex = m_queueFamily,
		.queueCount = 1,
		.pQueuePriorities = &qPriorities[0]
	};

	std::vector<const char*> DevExts = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
	};

	if (m_physDevices.Selected().m_features.geometryShader == VK_FALSE) {
		OGLDEV_ERROR0("The Geometry Shader is not supported!\n");
	}

	if (m_physDevices.Selected().m_features.tessellationShader == VK_FALSE) {
		OGLDEV_ERROR0("The Tessellation Shader is not supported!\n");
	}

	VkPhysicalDeviceFeatures DeviceFeatures = { 0 };
	DeviceFeatures.geometryShader = VK_TRUE;
	DeviceFeatures.tessellationShader = VK_TRUE;

	VkDeviceCreateInfo DeviceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &qInfo,
		.enabledLayerCount = 0,			// DEPRECATED
		.ppEnabledLayerNames = NULL,    // DEPRECATED
		.enabledExtensionCount = (u32)DevExts.size(),
		.ppEnabledExtensionNames = DevExts.data(),
		.pEnabledFeatures = &DeviceFeatures
	};

	VkResult res = vkCreateDevice(m_physDevices.Selected().m_physDevice, &DeviceCreateInfo, NULL, &m_device);
	CHECK_VK_RESULT(res, "Create device\n");

	printf("\nDevice created\n");
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


static VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR>& SurfaceFormats)
{
	for (int i = 0; i < SurfaceFormats.size(); i++) {
		if ((SurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB) &&
			(SurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
			return SurfaceFormats[i];
		}
	}

	return SurfaceFormats[0];
}

VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags,
	VkImageViewType ViewType, u32 LayerCount, u32 mipLevels)
{
	VkImageViewCreateInfo ViewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.image = Image,
		.viewType = ViewType,
		.format = Format,
		.components = {
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY
		},
		.subresourceRange = {
			.aspectMask = AspectFlags,
			.baseMipLevel = 0,
			.levelCount = mipLevels,
			.baseArrayLayer = 0,
			.layerCount = LayerCount
		}
	};

	VkImageView ImageView;
	VkResult res = vkCreateImageView(Device, &ViewInfo, NULL, &ImageView);
	CHECK_VK_RESULT(res, "vkCreateImageView");
	return ImageView;
}


void VulkanCore::CreateSwapChain()
{
	const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_physDevices.Selected().m_surfaceCaps;

	u32 NumImages = ChooseNumImages(SurfaceCaps);

	const std::vector<VkPresentModeKHR>& PresentModes = m_physDevices.Selected().m_presentModes;
	VkPresentModeKHR PresentMode = ChoosePresentMode(PresentModes);

	m_swapChainSurfaceFormat = ChooseSurfaceFormatAndColorSpace(m_physDevices.Selected().m_surfaceFormats);

	VkSwapchainCreateInfoKHR SwapChainCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.flags = 0,
		.surface = m_surface,
		.minImageCount = NumImages,
		.imageFormat = m_swapChainSurfaceFormat.format,
		.imageColorSpace = m_swapChainSurfaceFormat.colorSpace,
		.imageExtent = SurfaceCaps.currentExtent,
		.imageArrayLayers = 1,
		.imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &m_queueFamily,
		.preTransform = SurfaceCaps.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = PresentMode,
		.clipped = VK_TRUE
	};

	VkResult res = vkCreateSwapchainKHR(m_device, &SwapChainCreateInfo, NULL, &m_swapChain);
	CHECK_VK_RESULT(res, "vkCreateSwapchainKHR\n");

	printf("Swap chain created\n");

	uint NumSwapChainImages = 0;
	res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, NULL);
	CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");
	assert(NumImages <= NumSwapChainImages);

	printf("Requested %d images, created %d images\n", NumImages, NumSwapChainImages);

	m_images.resize(NumSwapChainImages);
	m_imageViews.resize(NumSwapChainImages);

	res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, m_images.data());
	CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");

	int LayerCount = 1;
	int MipLevels = 1;
	for (u32 i = 0; i < NumSwapChainImages; i++) {
		m_imageViews[i] = CreateImageView(m_device, m_images[i], m_swapChainSurfaceFormat.format,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, LayerCount, MipLevels);
	}
}


void VulkanCore::CreateCommandBufferPool()
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueFamilyIndex = m_queueFamily
	};

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

	printf("%d command buffers created\n", count);
}


void VulkanCore::FreeCommandBuffers(u32 Count, const VkCommandBuffer* pCmdBufs)
{
	m_queue.WaitIdle();
	vkFreeCommandBuffers(m_device, m_cmdBufPool, Count, pCmdBufs);
}


VkRenderPass VulkanCore::CreateSimpleRenderPass()
{
	VkAttachmentDescription AttachDesc = {
		.flags = 0,
		.format = m_swapChainSurfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference AttachRef = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription SubpassDesc = {
		.flags = 0,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount = 0,
		.pInputAttachments = NULL,
		.colorAttachmentCount = 1,
		.pColorAttachments = &AttachRef,
		.pResolveAttachments = NULL,
		.pDepthStencilAttachment = NULL,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = NULL
	};

	VkRenderPassCreateInfo RenderPassCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.attachmentCount = 1,
		.pAttachments = &AttachDesc,
		.subpassCount = 1,
		.pSubpasses = &SubpassDesc,
		.dependencyCount = 0,
		.pDependencies = NULL
	};

	VkRenderPass RenderPass;

	VkResult res = vkCreateRenderPass(m_device, &RenderPassCreateInfo, NULL, &RenderPass);
	CHECK_VK_RESULT(res, "vkCreateRenderPass\n");

	printf("Created a simple render pass\n");

	return RenderPass;
}


std::vector<VkFramebuffer> VulkanCore::CreateFramebuffers(VkRenderPass RenderPass) const
{
	std::vector<VkFramebuffer> frameBuffers;
	frameBuffers.resize(m_images.size());

	int WindowWidth, WindowHeight;
	GetFramebufferSize(WindowWidth, WindowHeight);

	VkResult res;

	for (uint i = 0; i < m_images.size(); i++) {

		VkFramebufferCreateInfo fbCreateInfo = {};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.renderPass = RenderPass;
		fbCreateInfo.attachmentCount = 1;
		fbCreateInfo.pAttachments = &m_imageViews[i];
		fbCreateInfo.width = WindowWidth;
		fbCreateInfo.height = WindowHeight;
		fbCreateInfo.layers = 1;

		res = vkCreateFramebuffer(m_device, &fbCreateInfo, NULL, &frameBuffers[i]);
		CHECK_VK_RESULT(res, "vkCreateFramebuffer\n");
	}

	printf("Framebuffers created\n");

	return frameBuffers;
}


void VulkanCore::DestroyFramebuffers(std::vector<VkFramebuffer>& Framebuffers)
{
	for (int i = 0; i < Framebuffers.size(); i++) {
		vkDestroyFramebuffer(m_device, Framebuffers[i], NULL);
	}
}


BufferAndMemory VulkanCore::CreateVertexBuffer(const void* pVertices, size_t Size)
{
	// Step 1: create the staging buffer
	VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags MemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
									 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	BufferAndMemory StagingVB = CreateBuffer(Size, Usage, MemProps);

	// Step 2: map the memory of the stage buffer
	void* pMem = NULL;
	VkDeviceSize Offset = 0;
	VkMemoryMapFlags Flags = 0;
	VkResult res = vkMapMemory(m_device, StagingVB.m_mem, Offset, 
		                       StagingVB.m_allocationSize, Flags, &pMem);
	CHECK_VK_RESULT(res, "vkMapMemory\n");

	// Step 3: copy the vertices to the stating buffer
	memcpy(pMem, pVertices, Size);

	// Step 4: unmap/release the mapped memory
	vkUnmapMemory(m_device, StagingVB.m_mem);

	// Step 5: create the final buffer
	Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	MemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	BufferAndMemory VB = CreateBuffer(Size, Usage, MemProps);

	// Step 6: copy the staging buffer to the final buffer
	CopyBuffer(VB.m_buffer, StagingVB.m_buffer, Size);

	// Step 7: release the resources of the staging buffer
	StagingVB.Destroy(m_device);

	return VB;
}


BufferAndMemory VulkanCore::CreateUniformBuffer(int Size)
{
	BufferAndMemory Buffer;

	Buffer = CreateBuffer(Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	return Buffer;
}


BufferAndMemory VulkanCore::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, 
	                                     VkMemoryPropertyFlags Properties)
{
	VkBufferCreateInfo vbCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = Size,
		.usage = Usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	BufferAndMemory Buf;

	// Step 1: create a buffer
	VkResult res = vkCreateBuffer(m_device, &vbCreateInfo, NULL, &Buf.m_buffer);
	CHECK_VK_RESULT(res, "vkCreateBuffer\n");
	printf("Buffer created\n");

	// Step 2: get the buffer memory requirements
	VkMemoryRequirements MemReqs = {};
	vkGetBufferMemoryRequirements(m_device, Buf.m_buffer, &MemReqs);
	printf("Buffer requires %d bytes\n", (int)MemReqs.size);

	Buf.m_allocationSize = MemReqs.size;

	// Step 3: get the memory type index
	u32 MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, Properties);
	printf("Memory type index %d\n", MemoryTypeIndex);

	// Step 4: allocate memory
	VkMemoryAllocateInfo MemAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = MemReqs.size,
		.memoryTypeIndex = MemoryTypeIndex
	};

	res = vkAllocateMemory(m_device, &MemAllocInfo, NULL, &Buf.m_mem);
	CHECK_VK_RESULT(res, "vkAllocateMemory error %d\n");

	// Step 5: bind memory
	res = vkBindBufferMemory(m_device, Buf.m_buffer, Buf.m_mem, 0);
	CHECK_VK_RESULT(res, "vkBindBufferMemory error %d\n");

	return Buf;
}

void VulkanCore::CopyBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size)
{
	BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy BufferCopy = {
		.srcOffset = 0,
		.dstOffset = 0,
		.size = Size
	};

	vkCmdCopyBuffer(m_copyCmdBuf, Src, Dst, 1, &BufferCopy);

	vkEndCommandBuffer(m_copyCmdBuf);

	m_queue.SubmitSync(m_copyCmdBuf);

	m_queue.WaitIdle();
}


u32 VulkanCore::GetMemoryTypeIndex(u32 MemTypeBitsMask, VkMemoryPropertyFlags ReqMemPropFlags)
{
	const VkPhysicalDeviceMemoryProperties& MemProps = m_physDevices.Selected().m_memProps;

	for (uint i = 0; i < MemProps.memoryTypeCount; i++) {
		const VkMemoryType& MemType = MemProps.memoryTypes[i];
		uint CurBitmask = (1 << i);
		bool IsCurMemTypeSupported = (MemTypeBitsMask & CurBitmask);
		bool HasRequiredMemProps = ((MemType.propertyFlags & ReqMemPropFlags) == ReqMemPropFlags);

		if (IsCurMemTypeSupported && HasRequiredMemProps) {
			return i;
		}
	}

	printf("Cannot find memory type for type %x requested mem props %x\n", MemTypeBitsMask, ReqMemPropFlags);
	exit(1);
	return -1;
}


void BufferAndMemory::Destroy(VkDevice Device)
{
	if (m_mem) {
		vkFreeMemory(Device, m_mem, NULL);
	}
	if (m_buffer) {
		vkDestroyBuffer(Device, m_buffer, NULL);
	}
}


void VulkanCore::GetFramebufferSize(int& Width, int& Height) const
{
	glfwGetWindowSize(m_pWindow, &Width, &Height);
}


std::vector<BufferAndMemory> VulkanCore::CreateUniformBuffers(size_t DataSize)
{
	std::vector<BufferAndMemory> UniformBuffers;

	UniformBuffers.resize(m_images.size());

	for (int i = 0; i < UniformBuffers.size(); i++) {
		UniformBuffers[i] = CreateUniformBuffer((int)DataSize);
	}

	return UniformBuffers;
}




void BufferAndMemory::Update(VkDevice Device, const void* pData, size_t Size)
{
	void* pMem = NULL;
	VkResult res = vkMapMemory(Device, m_mem, 0, Size, 0, &pMem);
	CHECK_VK_RESULT(res, "vkMapMemory");
	memcpy(pMem, pData, Size);
	vkUnmapMemory(Device, m_mem);
}


}
