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

#include "3rdparty/stb_image.h"

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

	if (m_depthEnabled) {
		for (int i = 0; i < m_depthImages.size(); i++) {
			m_depthImages[i].Destroy(m_device);
		}
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


void VulkanCore::Init(const char* pAppName, GLFWwindow* pWindow, bool DepthEnabled)
{
	m_pWindow = pWindow;
	m_depthEnabled = DepthEnabled;
	GetFramebufferSize(m_windowWidth, m_windowHeight);
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
	if (DepthEnabled) {
		CreateDepthResources();
	}
}


const VkImage& VulkanCore::GetImage(int Index) const
{
	if (Index >= m_images.size()) {
		OGLDEV_ERROR("Invalid image index %d\n", Index);
		exit(1);
	}

	return m_images[Index];
}


const VkImageView& VulkanCore::GetImageView(int Index) const
{
	if (Index >= m_imageViews.size()) {
		OGLDEV_ERROR("Invalid image view index %d\n", Index);
		exit(1);
	}

	return m_imageViews[Index];
}

const VkImageView& VulkanCore::GetDepthView(int Index) const
{
	if (Index >= m_depthImages.size()) {
		OGLDEV_ERROR("Invalid depth view index %d\n", Index);
		exit(1);
	}

	return m_depthImages[Index].m_view;
}


void VulkanCore::GetInstanceVersion()
{
	u32 InstanceVersion = 0;

	VkResult res = vkEnumerateInstanceVersion(&InstanceVersion);
	CHECK_VK_RESULT(res, "vkEnumerateInstanceVersion");

	m_instanceVersion.Major = VK_VERSION_MAJOR(InstanceVersion);
	m_instanceVersion.Minor = VK_VERSION_MINOR(InstanceVersion);
	m_instanceVersion.Patch = VK_VERSION_PATCH(InstanceVersion);

	printf("Vulkan loader supports version %d.%d.%d\n", 
		   m_instanceVersion.Major, m_instanceVersion.Minor, m_instanceVersion.Patch);
}


void VulkanCore::CreateInstance(const char* pAppName)
{
	GetInstanceVersion();

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
		.apiVersion = VK_MAKE_API_VERSION(0, m_instanceVersion.Major, m_instanceVersion.Minor, 0)
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

	bool DeviceSupportsDynamicRendering = m_physDevices.Selected().IsExtensionSupported(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
	
	bool Instance_is_1_3_or_more = (m_instanceVersion.Major > 1) || (m_instanceVersion.Minor >= 3);

	if (Instance_is_1_3_or_more && DeviceSupportsDynamicRendering) {
		printf("The Vulkan instance and device support dynamic rendering as a core feature\n");
	} else if (m_instanceVersion.Minor == 2) {
		if (DeviceSupportsDynamicRendering) {
			DevExts.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
		} else {
			printf("The system doesn't support dynamic rendering\n");
			exit(1);
		}
	} else {
		printf("The system doesn't support dynamic rendering\n");
		exit(1);
	}

	if (m_physDevices.Selected().m_features.geometryShader == VK_FALSE) {
		OGLDEV_ERROR0("The Geometry Shader is not supported!\n");
	}

	if (m_physDevices.Selected().m_features.tessellationShader == VK_FALSE) {
		OGLDEV_ERROR0("The Tessellation Shader is not supported!\n");
	}

	VkPhysicalDeviceFeatures DeviceFeatures = { 0 };
	DeviceFeatures.geometryShader = VK_TRUE;
	DeviceFeatures.tessellationShader = VK_TRUE;

	VkPhysicalDeviceDynamicRenderingFeaturesKHR DynamicRenderingFeature = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
		.pNext = NULL,
		.dynamicRendering = VK_TRUE
	};

	VkDeviceCreateInfo DeviceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &DynamicRenderingFeature,
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

	res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, m_images.data());
	CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");

	m_imageViews.resize(NumSwapChainImages);
	for (u32 i = 0; i < NumSwapChainImages; i++) {
		m_imageViews[i] = CreateImageView(m_device, m_images[i], m_swapChainSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}


void VulkanCore::CreateCommandBufferPool()
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
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
	VkAttachmentDescription ColorAttachment = {
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

	VkAttachmentReference ColorAttachRef = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkFormat DepthFormat = m_physDevices.Selected().m_depthFormat;

	VkAttachmentDescription DepthAttachment = {
		.flags = 0,
		.format = DepthFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkAttachmentReference DepthAttachmentRef = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription SubpassDesc = {
		.flags = 0,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount = 0,
		.pInputAttachments = NULL,
		.colorAttachmentCount = 1,
		.pColorAttachments = &ColorAttachRef,
		.pResolveAttachments = NULL,
		.pDepthStencilAttachment = m_depthEnabled ? &DepthAttachmentRef : NULL,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = NULL
	};

	std::vector< VkAttachmentDescription> Attachments;
	Attachments.push_back(ColorAttachment);

	if (m_depthEnabled) {
		Attachments.push_back(DepthAttachment);
	}

	VkRenderPassCreateInfo RenderPassCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.attachmentCount = (u32)Attachments.size(),
		.pAttachments = Attachments.data(),
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
	std::vector<VkFramebuffer> FrameBuffers;
	FrameBuffers.resize(m_images.size());

	for (uint i = 0; i < m_images.size(); i++) {
		std::vector<VkImageView> Attachments;
		Attachments.push_back(m_imageViews[i]);
		if (m_depthEnabled) {
			Attachments.push_back(m_depthImages[i].m_view);
		}

		VkFramebufferCreateInfo fbCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = RenderPass,
			.attachmentCount = (u32)Attachments.size(),
			.pAttachments = Attachments.data(),
			.width = (u32)m_windowWidth,
			.height = (u32)m_windowHeight,
			.layers = 1
		};

		VkResult res = vkCreateFramebuffer(m_device, &fbCreateInfo, NULL, &FrameBuffers[i]);
		CHECK_VK_RESULT(res, "vkCreateFramebuffer\n");
	}

	printf("Framebuffers created\n");

	return FrameBuffers;
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

	// Step 3: copy the vertices to the staging buffer
	memcpy(pMem, pVertices, Size);

	// Step 4: unmap/release the mapped memory
	vkUnmapMemory(m_device, StagingVB.m_mem);

	// Step 5: create the final buffer
	Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	MemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	BufferAndMemory VB = CreateBuffer(Size, Usage, MemProps);

	// Step 6: copy the staging buffer to the final buffer
	CopyBufferToBuffer(VB.m_buffer, StagingVB.m_buffer, Size);

	// Step 7: release the resources of the staging buffer
	StagingVB.Destroy(m_device);

	return VB;
}


BufferAndMemory VulkanCore::CreateUniformBuffer(size_t Size)
{
	BufferAndMemory Buffer;

	VkBufferUsageFlags Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags MemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
									 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	
	Buffer = CreateBuffer(Size, Usage, MemProps);

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
	VkMemoryRequirements MemReqs = { 0 };
	vkGetBufferMemoryRequirements(m_device, Buf.m_buffer, &MemReqs);
	printf("Buffer requires %d bytes\n", (int)MemReqs.size);

	Buf.m_allocationSize = MemReqs.size;

	// Step 3: get the memory type index
	u32 MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, Properties);
	//printf("Memory type index %d\n", MemoryTypeIndex);

	// Step 4: allocate memory
	VkMemoryAllocateInfo MemAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
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


void VulkanCore::CreateTexture(const char* pFilename, VulkanTexture& Tex)
{
	int ImageWidth = 0;
	int ImageHeight = 0;
	int ImageChannels = 0;

	stbi_set_flip_vertically_on_load(1);

	// Step #1: load the image pixels
	stbi_uc* pPixels = stbi_load(pFilename, &ImageWidth, &ImageHeight, &ImageChannels, STBI_rgb_alpha);

	if (!pPixels) {
		printf("Error loading texture from '%s'\n", pFilename);
		exit(1);
	}

	// Step #2: create the image object and populate it with pixels
	VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
	CreateTextureImageFromData(Tex, pPixels, ImageWidth, ImageHeight, Format);

	// Step #3: release the image pixels. We don't need them after this point
	stbi_image_free(pPixels);

	// Step #4: create the image view
	VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	Tex.m_view = CreateImageView(m_device, Tex.m_image, Format, AspectFlags);

	VkFilter MinFilter = VK_FILTER_LINEAR;
	VkFilter MaxFilter = VK_FILTER_LINEAR;
	VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	// Step #5: create the texture sampler
	Tex.m_sampler = CreateTextureSampler(m_device, MinFilter, MaxFilter, AddressMode);

	printf("Texture from '%s' created\n", pFilename);
}


void VulkanCore::CreateTextureFromData(const void* pPixels, int ImageWidth, int ImageHeight, VulkanTexture& Tex)
{
	// Step #1: create the image object and populate it with pixels
	VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
	CreateTextureImageFromData(Tex, pPixels, ImageWidth, ImageHeight, Format);

	// Step #2: create the image view
	VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	Tex.m_view = CreateImageView(m_device, Tex.m_image, Format, AspectFlags);

	VkFilter MinFilter = VK_FILTER_LINEAR;
	VkFilter MaxFilter = VK_FILTER_LINEAR;
	VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	// Step #3: create the texture sampler
	Tex.m_sampler = CreateTextureSampler(m_device, MinFilter, MaxFilter, AddressMode);

	printf("Texture from data created\n");
}


void VulkanTexture::Destroy(VkDevice Device)
{
	vkDestroySampler(Device, m_sampler, NULL);
	vkDestroyImageView(Device, m_view, NULL);
	vkDestroyImage(Device, m_image, NULL);
	vkFreeMemory(Device, m_mem, NULL);
}


void VulkanCore::CreateTextureImageFromData(VulkanTexture& Tex, const void* pPixels, 
											u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat)
{
	VkImageUsageFlagBits Usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
														VK_IMAGE_USAGE_SAMPLED_BIT);
	VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	CreateImage(Tex, ImageWidth, ImageHeight, TexFormat, Usage, PropertyFlags);

	UpdateTextureImage(Tex, ImageWidth, ImageHeight, TexFormat, pPixels);
}


void VulkanCore::CreateImage(VulkanTexture& Tex, u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat,
	                         VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags)
{
	VkImageCreateInfo ImageInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = TexFormat,
		.extent = VkExtent3D {.width = ImageWidth, .height = ImageHeight, .depth = 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = UsageFlags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	// Step #1: create the image object
	VkResult res = vkCreateImage(m_device, &ImageInfo, NULL, &Tex.m_image);
	CHECK_VK_RESULT(res, "vkCreateImage error");

	// Step 2: get the buffer memory requirements
	VkMemoryRequirements MemReqs = { 0 };
	vkGetImageMemoryRequirements(m_device, Tex.m_image, &MemReqs);
	printf("Image requires %d bytes\n", (int)MemReqs.size);

	// Step 3: get the memory type index
	u32 MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, PropertyFlags);
	printf("Memory type index %d\n", MemoryTypeIndex);

	// Step 4: allocate memory
	VkMemoryAllocateInfo MemAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize = MemReqs.size,
		.memoryTypeIndex = MemoryTypeIndex
	};

	res = vkAllocateMemory(m_device, &MemAllocInfo, NULL, &Tex.m_mem);
	CHECK_VK_RESULT(res, "vkAllocateMemory error");

	// Step 5: bind memory
	res = vkBindImageMemory(m_device, Tex.m_image, Tex.m_mem, 0);
	CHECK_VK_RESULT(res, "vkBindBufferMemory error %d\n");
}


void VulkanCore::UpdateTextureImage(VulkanTexture& Tex, u32 ImageWidth, u32 ImageHeight, 
								    VkFormat TexFormat, const void* pPixels)
{
	int BytesPerPixel = GetBytesPerTexFormat(TexFormat);

	VkDeviceSize LayerSize = ImageWidth * ImageHeight * BytesPerPixel;
	int LayerCount = 1;
	VkDeviceSize ImageSize = LayerCount * LayerSize;

	VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
									   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	BufferAndMemory StagingTex = CreateBuffer(ImageSize, Usage, Properties);

	StagingTex.Update(m_device, pPixels, ImageSize);

	TransitionImageLayout(Tex.m_image, TexFormat, 
						  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	
	CopyBufferToImage(Tex.m_image, StagingTex.m_buffer, ImageWidth, ImageHeight);
	
	TransitionImageLayout(Tex.m_image, TexFormat, 
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	StagingTex.Destroy(m_device);
}




void VulkanCore::TransitionImageLayout(VkImage& Image, VkFormat Format, 
									   VkImageLayout OldLayout, VkImageLayout NewLayout)
{
	BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	ImageMemBarrier(m_copyCmdBuf, Image, Format, OldLayout, NewLayout);

	SubmitCopyCommand();
}


void VulkanCore::CopyBufferToBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size)
{
	BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy BufferCopy = {
		.srcOffset = 0,
		.dstOffset = 0,
		.size = Size
	};

	vkCmdCopyBuffer(m_copyCmdBuf, Src, Dst, 1, &BufferCopy);

	SubmitCopyCommand();
}


void VulkanCore::CopyBufferToImage(VkImage Dst, VkBuffer Src, u32 ImageWidth, u32 ImageHeight)
{
	BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferImageCopy BufferImageCopy = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = VkImageSubresourceLayers {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
		.imageOffset = VkOffset3D {.x = 0, .y = 0, .z = 0 },
		.imageExtent = VkExtent3D {.width = ImageWidth, .height = ImageHeight, .depth = 1 }
	};

	vkCmdCopyBufferToImage(m_copyCmdBuf, Src, Dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
						   1, &BufferImageCopy);
	SubmitCopyCommand();
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


std::vector<BufferAndMemory> VulkanCore::CreateUniformBuffers(size_t Size)
{
	std::vector<BufferAndMemory> UniformBuffers;

	UniformBuffers.resize(m_images.size());

	for (int i = 0; i < UniformBuffers.size(); i++) {
		UniformBuffers[i] = CreateUniformBuffer(Size);
	}

	return UniformBuffers;
}


void VulkanCore::SubmitCopyCommand()
{
	vkEndCommandBuffer(m_copyCmdBuf);

	m_queue.SubmitSync(m_copyCmdBuf);

	m_queue.WaitIdle();
}


void VulkanCore::CreateDepthResources()
{
	int NumSwapChainImages = (int)m_images.size();

	m_depthImages.resize(NumSwapChainImages);

    VkFormat DepthFormat = m_physDevices.Selected().m_depthFormat;

	for (int i = 0; i < NumSwapChainImages; i++) {
		VkImageUsageFlagBits Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		CreateImage(m_depthImages[i], m_windowWidth, m_windowHeight, DepthFormat, 
					Usage, PropertyFlags);

		VkImageLayout OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout NewLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		TransitionImageLayout(m_depthImages[i].m_image, DepthFormat, OldLayout, NewLayout);

		m_depthImages[i].m_view = CreateImageView(m_device, m_depthImages[i].m_image, 
												  DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
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
