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
#include <array>

#include <glm/glm.hpp>
#include <glm/ext.hpp>


#include <assimp/Importer.hpp> 
#include <assimp/scene.h>      
#include <assimp/postprocess.h> 

#include "ogldev_types.h"
#include "ogldev_util.h"
#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_wrapper.h"
#include "3rdparty/stb_image.h"

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
		printf("%llux ", pCallbackData->pObjects[i].objectHandle);
	}

	printf("\n");

	return VK_FALSE;  // The calling function should not be aborted
}


static VkFormat FindSupportedFormat(const PhysicalDevice& PhyiscalDevice, const std::vector<VkFormat>& Candidates,
	VkImageTiling Tiling, VkFormatFeatureFlags Features)
{
	for (int i = 0; i < Candidates.size(); i++) {
		VkFormat Format = Candidates[i];
		VkFormatProperties Props;
		vkGetPhysicalDeviceFormatProperties(PhyiscalDevice.m_physDevice, Format, &Props);

		if (Tiling == VK_IMAGE_TILING_LINEAR && (Props.linearTilingFeatures & Features) == Features) {
			return Format;
		}
		else if (Tiling == VK_IMAGE_TILING_OPTIMAL && (Props.optimalTilingFeatures & Features) == Features) {
			return Format;
		}
	}

	printf("failed to find supported format!\n");
	exit(0);
}


static VkFormat FindDepthFormat(const PhysicalDevice& PhyiscalDevice)
{
	std::vector<VkFormat> FormatCandidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

	return FindSupportedFormat(PhyiscalDevice, FormatCandidates,
		                       VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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

	DestroyTexture(m_depthTexture);

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
	m_queue.Init(m_device, m_queueFamily, 0);
	CreateCommandBufferPool();
	CreateSwapChain();	
	m_queue.SetSwapChain(m_swapChain);
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

	VkFormat DepthFormat = FindDepthFormat(m_physDevices.Selected());

	CreateImage(m_depthTexture, SurfaceCaps.currentExtent.width, SurfaceCaps.currentExtent.height, DepthFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, 1);

	m_depthTexture.m_view = CreateImageView(m_device, m_depthTexture.m_image, DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT,
											VK_IMAGE_VIEW_TYPE_2D, 1, 1);

	TransitionImageLayout(m_depthTexture.m_image, DepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, 
						  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, LayerCount, 1);
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


VkRenderPass VulkanCore::CreateSimpleRenderPass(bool DepthEnabled, bool ClearColor, bool ClearDepth, RenderPassType RenderPassType) const
{
	bool OffscreenInt = RenderPassType & RenderPassTypeOffscreenInternal;
	bool First = RenderPassType & RenderPassTypeFirst;
	bool Last = RenderPassType & RenderPassTypeLast;

	VkImageLayout InitialLayout = First ? VK_IMAGE_LAYOUT_UNDEFINED :
										  (OffscreenInt ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL :
											              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	printf("Creating a simple render pass\n");
	printf("Initial layout %x\n", InitialLayout);

	VkAttachmentDescription ColorAttachDesc = {
		.flags = 0,
		.format = m_swapChainSurfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = OffscreenInt ? VK_ATTACHMENT_LOAD_OP_LOAD : (ClearColor ?
								 VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD),
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = InitialLayout,
		.finalLayout = Last ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkAttachmentReference ColorAttachRef = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkAttachmentDescription DepthAttachDesc = { 0 };
	VkAttachmentReference DepthAttachRef = { 0 };

	if (DepthEnabled) {
		DepthAttachDesc = {
			.flags = 0,
			.format = DepthEnabled ? FindDepthFormat(m_physDevices.Selected()) : VK_FORMAT_D32_SFLOAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = OffscreenInt ? VK_ATTACHMENT_LOAD_OP_LOAD : 
									 (ClearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD),
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = ClearDepth ? VK_IMAGE_LAYOUT_UNDEFINED : 
							              (OffscreenInt ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : 
														  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		DepthAttachRef = {
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};
	}	

	if (RenderPassType & RenderPassTypeOffscreen) {
		ColorAttachDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	VkSubpassDependency Dependencies = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dependencyFlags = 0
	};

	VkSubpassDescription SubpassDesc = {
		.flags = 0,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount = 0,
		.pInputAttachments = NULL,
		.colorAttachmentCount = 1,
		.pColorAttachments = &ColorAttachRef,
		.pResolveAttachments = NULL,
		.pDepthStencilAttachment = DepthEnabled ? &DepthAttachRef : NULL,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = NULL
	};

	std::array<VkAttachmentDescription, 2> Attachments = { ColorAttachDesc, DepthAttachDesc };

	VkRenderPassCreateInfo RenderPassCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.attachmentCount = (u32)(DepthEnabled ? 2 : 1),
		.pAttachments = Attachments.data(),
		.subpassCount = 1,
		.pSubpasses = &SubpassDesc,
		.dependencyCount = 1,
		.pDependencies = &Dependencies
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
		std::array<VkImageView, 2> Attachments = { m_imageViews[i], m_depthTexture.m_view };

		VkFramebufferCreateInfo fbCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = RenderPass,
			.attachmentCount = (u32)Attachments.size(),
			.pAttachments = Attachments.data(),
			.width = (u32)WindowWidth,
			.height = (u32)WindowHeight,
			.layers = 1
		};

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
	VkBuffer StagingVB;
	VkDeviceMemory StagingVBMem;
	VkDeviceSize AllocationSize = CreateBuffer(Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingVB, StagingVBMem);

	void* MappedMemAddr = NULL;
	VkResult res = vkMapMemory(m_device, StagingVBMem, 0, AllocationSize, 0, &MappedMemAddr);
	memcpy(MappedMemAddr, pVertices, Size);
	vkUnmapMemory(m_device, StagingVBMem);

	BufferAndMemory VB;
	AllocationSize = CreateBuffer(Size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VB.m_buffer, VB.m_mem);

	CopyBuffer(VB.m_buffer, StagingVB, Size);

	vkDestroyBuffer(m_device, StagingVB, NULL);
	vkFreeMemory(m_device, StagingVBMem, NULL);

	return VB;
}


BufferAndMemory VulkanCore::CreateUniformBuffer(int Size)
{
	BufferAndMemory Buffer;

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


void VulkanCore::CreateTexture(const char* pFilename, VulkanTexture& Tex)
{
	int ImageWidth = 0;
	int ImageHeight = 0;
	int ImageChannels = 0;

	stbi_uc* pPixels = stbi_load(pFilename, &ImageWidth, &ImageHeight, &ImageChannels, STBI_rgb_alpha);

	if (!pPixels) {
		printf("Error loading texture from '%s'\n", pFilename);
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

	Tex.m_view = CreateImageView(m_device, Tex.m_image, Format, AspectFlags, ViewType, LayerCount, MipLevels);

	VkFilter MinFilter = VK_FILTER_LINEAR;
	VkFilter MaxFilter = VK_FILTER_LINEAR;
	VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	CreateTextureSampler(m_device, &Tex.m_sampler, MinFilter, MaxFilter, AddressMode);

	printf("Texture from '%s' created\n", pFilename);
}


void VulkanCore::DestroyTexture(VulkanTexture& Tex)
{
	vkDestroySampler(m_device, Tex.m_sampler, NULL);
	vkDestroyImageView(m_device, Tex.m_view, NULL);
	vkDestroyImage(m_device, Tex.m_image, NULL);
	vkFreeMemory(m_device, Tex.m_mem, NULL);
}


void VulkanCore::CreateTextureImageFromData(VulkanTexture& Tex, const void* pPixels, u32 ImageWidth, u32 ImageHeight,
											VkFormat TexFormat, u32 LayerCount, VkImageCreateFlags CreateFlags)
{
	CreateImage(Tex, ImageWidth, ImageHeight, TexFormat, VK_IMAGE_TILING_OPTIMAL, 
			    (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, CreateFlags, 1);

	UpdateTextureImage(Tex, ImageWidth, ImageHeight, TexFormat, LayerCount, pPixels, VK_IMAGE_LAYOUT_UNDEFINED);
}


void VulkanCore::CreateImage(VulkanTexture& Tex, u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat, VkImageTiling ImageTiling,
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


void VulkanCore::UpdateTextureImage(VulkanTexture& Tex, u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat, u32 LayerCount, const void* pPixels, VkImageLayout SourceImageLayout)
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
	VkCommandBuffer CmdBuf = CreateAndBeginSingleUseCommand();

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
	VkCommandBuffer CmdBuf = CreateAndBeginSingleUseCommand();

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
void VulkanCore::CopyBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size)
{
	BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy bufferCopy = {};
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = Size;
	vkCmdCopyBuffer(m_copyCmdBuf, Src, Dst, 1, &bufferCopy);

	vkEndCommandBuffer(m_copyCmdBuf);

	m_queue.SubmitSync(m_copyCmdBuf);

	m_queue.WaitIdle();
}


u32 VulkanCore::GetMemoryTypeIndex(u32 memTypeBits, VkMemoryPropertyFlags reqMemPropFlags)
{
	const VkPhysicalDeviceMemoryProperties& MemProps = m_physDevices.Selected().m_memProps;

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


std::vector<BufferAndMemory> VulkanCore::CreateUniformBuffers(size_t DataSize)
{
	std::vector<BufferAndMemory> UniformBuffers;

	UniformBuffers.resize(m_images.size());

	for (int i = 0; i < UniformBuffers.size(); i++) {
		UniformBuffers[i] = CreateUniformBuffer((int)DataSize);
	}

	return UniformBuffers;
}


VkCommandBuffer VulkanCore::CreateAndBeginSingleUseCommand()
{
	VkCommandBuffer CmdBuf;

	CreateCommandBuffers(1, &CmdBuf);

	BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	return CmdBuf;
}


void VulkanCore::EndSingleTimeCommands(VkCommandBuffer CmdBuf)
{
	vkEndCommandBuffer(CmdBuf);

	m_queue.SubmitSync(CmdBuf);

	m_queue.WaitIdle();

	vkFreeCommandBuffers(m_device, m_cmdBufPool, 1, &CmdBuf);
}


SimpleMesh VulkanCore::LoadSimpleMesh(const char* pFilename)
{
	Assimp::Importer Importer;
	const aiScene* pScene = Importer.ReadFile(pFilename, aiProcess_Triangulate);

	if (!pScene) {
		printf("Error parsing '%s': '%s'\n", pFilename, Importer.GetErrorString());
		exit(1);
	} 
	
	if (!pScene->HasMeshes()) {
		printf("File '%s' hash no meshes\n", pFilename);
		exit(255);
	}

	const aiMesh* pMesh = pScene->mMeshes[0];

	SimpleMesh Mesh(m_device);
	Mesh.m_numVertices = pMesh->mNumVertices;
	Mesh.m_numIndices = pMesh->mNumFaces * 3;

	struct VertexData {
		glm::vec3 pos;
		glm::vec2 tc;
	};

	std::vector<VertexData> Vertices;

	Vertices.resize(pMesh->mNumVertices);

	for (int i = 0; i != pMesh->mNumVertices; i++) {
		aiVector3D v = pMesh->mVertices[i];
		glm::vec3 Pos(v.x, v.z, v.y);

		glm::vec2 TexCoords(0.0f, 0.0f);
		if (pMesh->mTextureCoords[0]) {
			aiVector3D t = pMesh->mTextureCoords[0][i];
			TexCoords = glm::vec2(t.x, 1.0f - t.y);
		}		
		
		Vertices[i] = {.pos = Pos, .tc = TexCoords};
	}

	std::vector<unsigned int> Indices;
	Indices.resize(pMesh->mNumFaces * 3);

	for (int i = 0; i != pMesh->mNumFaces; i++) {
		for (int j = 0; j != 3; j++) {
			Indices[i * 3 + j] = pMesh->mFaces[i].mIndices[j];
		}
	}

	//aiReleaseImport(scene);

	Mesh.m_vertexBufferSize = sizeof(VertexData) * Vertices.size();
	Mesh.m_indexBufferSize = sizeof(u32) * Indices.size();

	Mesh.m_vb = CreateVertexBuffer(Vertices.data(), Mesh.m_vertexBufferSize);
	Mesh.m_ib = CreateVertexBuffer(Indices.data(), Mesh.m_indexBufferSize);

	return Mesh;
}


void VulkanCore::GetFramebufferSize(int& Width, int& Height) const
{
	glfwGetWindowSize(m_pWindow, &Width, &Height);
}


void SimpleMesh::Destroy()
{
	if (m_device) {
		m_vb.Destroy(m_device);
		m_ib.Destroy(m_device);
	} else {
		printf("SimpleMesh destroyed but no device\n");
		return;
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


void BufferAndMemory::Destroy(VkDevice Device)
{
	if (m_mem) {
		vkFreeMemory(Device, m_mem, NULL);
	}

	if (m_buffer) {
		vkDestroyBuffer(Device, m_buffer, NULL);
	}
}

}

