/*

		Copyright 2026 Etay Meiri

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

	Vulkan For Beginners - 
		Tutorial #33:
*/

#include <array>
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imGuIZMOquat.h"

#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_wrapper.h"
#include "ogldev_vulkan_shader.h"
#include "ogldev_vulkan_glfw.h"
#include "ogldev_vulkan_model.h"
#include "ogldev_glm_camera.h"
#include "ogldev_vulkan_imgui.h"
#include "ogldev_vulkan_big_texture_array.h"
#include "Int/model_desc.h"
#include "lighting_program.h"
#include "tone_mapping_program.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define MAX_TEXTURES 4096

#define BIG_TEXTURE_ARRAY_BINDING 0

#define APP_NAME "Tutorial 33"

struct ModelContext {
	OgldevVK::VkModel* m_pModel = NULL;
	std::vector<VkDescriptorSet> m_descSets;
	std::vector<OgldevVK::BufferAndMemory> m_uniformBuffersVS;
	std::vector<OgldevVK::BufferAndMemory> m_uniformBuffersFS;
    int m_baseTextureIndex = -1;

	void Destroy(VkDevice Device)
	{
		for (OgldevVK::BufferAndMemory& ub : m_uniformBuffersVS) {
			ub.Destroy(Device);
		}

		for (OgldevVK::BufferAndMemory& ub : m_uniformBuffersFS) {
			ub.Destroy(Device);
		}
		delete m_pModel;
	}
};

struct ModelConfig {
	std::string Path;
	glm::vec3 Pos = glm::vec3(0.0);
    float Scale = 1.0f;
};

static std::vector<ModelConfig> Models = {
//	{ "G:/Models/McGuire/San_Miguel/san-miguel.obj", glm::vec3(0.0f), 1.0f }
//	{ "../../Content/crytek_sponza/sponza.obj", glm::vec3(0.0f), 0.01f }
//	,{ "../../Content/vintage_cabinet_01/vintage_cabinet_01_4k.gltf", glm::vec3(-8.0f, 0.0f, -1.5f), 1.0f}
	{ "../../Content/box.obj", glm::vec3(2.0f, 0.5f, -1.5f), 0.25f}
//	,{ "../../Content/antique_ceramic_vase_01_4k.blend/antique_ceramic_vase_01_4k.obj", glm::vec3(-4.0f, 0.0f, -1.5f), 2.0f}
//	,{ "../../Content/Stanford/stanford_dragon_pbr/scene.gltf", glm::vec3(0.0f, 0.0f, -1.5f), 0.02f }
};



class VulkanApp : public OgldevVK::GLFWCallbacks
{
public:

	VulkanApp(int WindowWidth, int WindowHeight)
	{
		m_windowWidth = WindowWidth;
		m_windowHeight = WindowHeight;
	}

	~VulkanApp()
	{
		if (m_vkCore.GetDevice() != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(m_vkCore.GetDevice());
		}

		m_imGUIRenderer.Destroy();

        for (int MeshIndex = 0; MeshIndex < m_modelContexts.size(); MeshIndex++) {
			for (MeshCmdBufs& v : m_cmdBufs[MeshIndex]) {
				m_vkCore.FreeCommandBuffers((u32)v.BaseMeshDraw.size(), v.BaseMeshDraw.data());
			}
        }
        
		for (int i = 0; i < (int)m_modelContexts.size(); i++) {
            m_modelContexts[i].Destroy(m_device);
		}

		vkDestroyShaderModule(m_device, m_vs, NULL);
		vkDestroyShaderModule(m_device, m_fs, NULL);

		for (OgldevVK::LightingProgram& p : m_pipelines) {
			p.Destroy();
		}
        
        m_toneMappingPipeline.Destroy();

        for (int i = 0; i < (int)m_offlineImages.size(); i++) {
            m_offlineImages[i].Destroy(m_device);
        }

        m_bigTextureArray.Destroy();		

		vkDestroyDescriptorPool(m_device, m_descPool, NULL);			

		glfwTerminate();
	}


	void Init(const char* pAppName)
	{
		m_pWindow = OgldevVK::glfw_vulkan_init(WINDOW_WIDTH, WINDOW_HEIGHT, pAppName);

        m_vkCore.Init(pAppName, m_pWindow, (OgldevVK::InitFlags)(OgldevVK::OGLDEV_VK_INIT_COMPUTE_ENABLED));
		m_device = m_vkCore.GetDevice();
		m_numImages = m_vkCore.GetNumImages();
		m_pQueue = m_vkCore.GetQueue();
		CreateShaders();
		CreateDescriptorPool();
		InitBigTextureArray();
		CreateOfflineImages();
		CreatePipeline();
		CreateMeshes();
		CreateCommandBuffers();
		RecordCommandBuffers();
		DefaultCreateCameraPers();
		// The object is ready to receive callbacks
		OgldevVK::glfw_vulkan_set_callbacks(m_pWindow, this);
		m_imGUIRenderer.Init(&m_vkCore);
	}


	void RenderScene()
	{
		u32 ImageIndex = m_pQueue->AcquireNextImage();

		for (int MeshIndex = 0; MeshIndex < m_modelContexts.size(); MeshIndex++) {
			UpdateUniformBuffers(MeshIndex, ImageIndex);
		}

		std::vector<VkCommandBuffer> SubmissionCmdBufs;

		// 1. Geometry Pass
		for (size_t MeshIndex = 0; MeshIndex < m_modelContexts.size(); MeshIndex++) {
			SubmissionCmdBufs.push_back(m_cmdBufs[MeshIndex][m_lightingMode].BaseMeshDraw[ImageIndex]);
		}

		// 2. Resolve Post-Processing and Presentation Chains
		if (m_enableToneMapping) {
			SubmissionCmdBufs.push_back(m_toneMappingCmdBufs[ImageIndex]);

			if (m_showGui) {
				UpdateGUI();
				VkCommandBuffer ImGUICmdBuf = m_imGUIRenderer.PrepareCommandBuffer(ImageIndex);
				SubmissionCmdBufs.push_back(ImGUICmdBuf);
			} else {
				SubmissionCmdBufs.push_back(m_toneMappingTransitionCmdBufs[ImageIndex]);
			}
		} else {
			// Fallback Blit Path
			SubmissionCmdBufs.push_back(m_fallbackCopyCmdBufs[ImageIndex]);

			if (m_showGui) {
				UpdateGUI();
				VkCommandBuffer ImGUICmdBuf = m_imGUIRenderer.PrepareCommandBuffer(ImageIndex);
				SubmissionCmdBufs.push_back(ImGUICmdBuf);
			} else {
				SubmissionCmdBufs.push_back(m_transitionCmdBufs[ImageIndex]);
			}
		}

		m_pQueue->SubmitAsync(SubmissionCmdBufs);
		m_pQueue->Present(ImageIndex);
	}

	
	void Key(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods)
	{
		bool Handled = true;
		bool Press = Action != GLFW_RELEASE;

		switch (Key) {
	    case GLFW_KEY_SPACE:
	        if (Press) {
	            m_showGui = !m_showGui;
	        }
	        break;
		
		case GLFW_KEY_ESCAPE:
		case GLFW_KEY_Q:
			if (Press) {
				glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
			}
			break;

		case GLFW_KEY_C:
			m_pGameCamera->Print();
			break;

		default:
			Handled = false;
		}

		if (!Handled) {
			Handled = GLFWCameraHandler(m_pGameCamera->m_movement, Key, Action, Mods);
		}
	}
	
	
	void MouseMove(GLFWwindow* pWindow, double x, double y)
	{
		m_pGameCamera->SetMousePos((float)x, (float)y);
	}


	void MouseButton(GLFWwindow* pWindow, int Button, int Action, int Mods)
	{
		if (m_showGui && IsMouseControlledByImGUI()) {
			return;
		}
		
		m_pGameCamera->HandleMouseButton(Button, Action, Mods);
	}
	
	
	void Execute()
	{
		float CurTime = (float)glfwGetTime();

		int Frames = 0;
		float FPSTime = 0.0f;
		while (!glfwWindowShouldClose(m_pWindow)) {
			float Time = (float)glfwGetTime();
			float dt = Time - CurTime;
			m_pGameCamera->Update(dt);
			RenderScene();
			CurTime = Time;
			glfwPollEvents();

			Frames++;
			FPSTime += dt;

			if (FPSTime >= 1.0f) {
				//printf("%d\n", Frames);
				char Title[256];
				snprintf(Title, sizeof(Title), "%s : FPS %d\n", APP_NAME, Frames);
				glfwSetWindowTitle(m_pWindow, Title);
				FPSTime = 0.0f;
				Frames = 0;
			}
		}
	}


private:

	void DefaultCreateCameraPers()
	{
		float FOV = 45.0f;
		float zNear = 0.1f;
		float zFar = 1500.0f;

		DefaultCreateCameraPers(FOV, zNear, zFar);
	}


	void DefaultCreateCameraPers(float FOV, float zNear, float zFar)
	{
		if ((m_windowWidth == 0) || (m_windowHeight == 0)) {
			printf("Invalid window dims: width %d height %d\n", m_windowWidth, m_windowHeight);
			exit(1);
		}

		if (m_pGameCamera) {
			printf("Camera already initialized\n");
			exit(1);
		}

		PersProjInfo persProjInfo = { FOV, (float)m_windowWidth, (float)m_windowHeight,
									  zNear, zFar };
		
		glm::vec3 Pos(10.0f, 2.4f, -0.17f);
		glm::vec3 Target(-1.0f, -0.025f, 0.0f);
		glm::vec3 Up(0.0, 1.0f, 0.0f);

		m_pGameCamera = new GLMCameraFirstPerson(Pos, Target, Up, persProjInfo);
		m_pGameCamera->m_maxSpeed = 1.5f;
	}


	void CreateCommandBuffers()
	{		
        m_cmdBufs.resize(m_modelContexts.size());

        for (int i = 0; i < m_modelContexts.size(); i++) {
			m_cmdBufs[i].resize(OgldevVK::NUM_LIGHTING_MODES);

			for (MeshCmdBufs& v : m_cmdBufs[i]) {
				v.BaseMeshDraw.resize(m_numImages);
				m_vkCore.CreateCommandBuffers(m_numImages, v.BaseMeshDraw.data());
			}
        }

		printf("Created command buffers\n");
	}


	void CreateDescriptorPool()
	{
		u32 TextureCount = MAX_TEXTURES * 4;
		u32 UniformBufferCount = 50;
		u32 StorageBufferCount = 50;
		u32 StorageImageCount = m_numImages;
		u32 MaxSets = (m_numImages * (u32)Models.size() * (OgldevVK::NUM_LIGHTING_MODES + 1)) + m_numImages; // +1 for the global texture array descriptor set

		m_descPool = m_vkCore.CreateDescPool(TextureCount, UniformBufferCount, StorageBufferCount, StorageImageCount, MaxSets);
	}


	void InitBigTextureArray()
	{
        m_bigTextureArray.Init(m_vkCore.GetDevice(), m_descPool, MAX_TEXTURES, BIG_TEXTURE_ARRAY_BINDING);
	}


	void CreateOfflineImages()
	{
        m_offlineImages.resize(m_numImages);

		VkExtent2D SwapChainExtent = m_vkCore.GetSwapChainExtent();
		VkFormat SwapChainFormat = m_vkCore.GetSwapChainFormat();
        VkFormat DepthFormat = m_vkCore.GetDepthFormat();
		VkImageUsageFlags ColorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
								  	   VK_IMAGE_USAGE_SAMPLED_BIT |
									   VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
									   VK_IMAGE_USAGE_STORAGE_BIT;
		VkImageUsageFlags DepthUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        for (int i = 0; i < (int)m_offlineImages.size(); i++) {
            m_vkCore.CreateTexture(m_offlineImages[i].m_color, SwapChainExtent.width, SwapChainExtent.height, 
								   ColorUsage, SwapChainFormat, false);
			
            m_vkCore.CreateTexture(m_offlineImages[i].m_depth, SwapChainExtent.width, SwapChainExtent.height, 
								   DepthUsage, DepthFormat, false);

        }
	}


	void CreateMeshes()
	{
        m_modelContexts.resize(Models.size());

        std::vector<OgldevVK::ModelDesc> ModelDescs(m_modelContexts.size());
		
		for (int i = 0; i < (int)m_modelContexts.size(); i++) {
			m_modelContexts[i].m_pModel = new OgldevVK::VkModel();
			m_modelContexts[i].m_pModel->Init(&m_vkCore, true, false);
			m_modelContexts[i].m_pModel->LoadAssimpModel(Models[i].Path);
            CreateUniformBuffers(i);
            CreateDescriptorSets(i, ModelDescs[i]);
		}

		m_toneMappingPipeline.AllocDescSets(m_toneMappingDescSets);
        m_toneMappingPipeline.UpdateDescriptorSets(m_toneMappingDescSets, m_vkCore.GetImageViews(), m_offlineImages);

        UpdateBaseTextureIndices(ModelDescs);

		m_bigTextureArray.CreateTextureArray(ModelDescs);
	}


    void UpdateBaseTextureIndices(std::vector<OgldevVK::ModelDesc>& ModelDescs)
    {
        u32 TotalTextureCount = 0;
        for (int i = 0; i < (int)m_modelContexts.size(); i++) {
            m_modelContexts[i].m_baseTextureIndex = TotalTextureCount;
			const OgldevVK::ModelDesc& md = ModelDescs[i];
			TotalTextureCount += (u32)md.m_materials.size();
        }
    }


	void CreateShaders()
	{
		m_vs = OgldevVK::CreateShaderModuleFromText(m_device, "test.vert");

		m_fs = OgldevVK::CreateShaderModuleFromText(m_device, "test.frag");
	}


	void CreatePipeline()
	{
		for (int i = 0; i < OgldevVK::NUM_LIGHTING_MODES; i++) {
			m_pipelines[i].Init(m_vkCore, m_descPool, m_bigTextureArray.GetDescSetLayout(), 
				                m_bigTextureArray.GetDescSet(), m_vs, m_fs, (OgldevVK::LIGHTING_MODE)i);
		}

        m_toneMappingPipeline.Init(m_vkCore, m_descPool);
	}


	void CreateUniformBuffers(int MeshIndex)
	{		
        // VS uniform buffers (actually using SSBOs)
        size_t NumMeshes = m_modelContexts[MeshIndex].m_pModel->GetNumMeshes();
		size_t UniformBufferSizeVS = OgldevVK::LightingProgram::GetUniformBufferSizeVS(NumMeshes);

        m_modelContexts[MeshIndex].m_uniformBuffersVS.resize(m_numImages);

        for (int i = 0; i < m_numImages; i++) {
			m_modelContexts[MeshIndex].m_uniformBuffersVS[i] = m_vkCore.CreateSSBO(UniformBufferSizeVS);
        }

        // FS uniform buffers
		size_t UniformBufferSizeFS = OgldevVK::LightingProgram::GetUniformBufferSizeFS();
		m_modelContexts[MeshIndex].m_uniformBuffersFS = m_vkCore.CreateUniformBuffers(UniformBufferSizeFS);
	}


	void CreateDescriptorSets(int MeshIndex, OgldevVK::ModelDesc& md)
	{		
		m_modelContexts[MeshIndex].m_pModel->UpdateModelDesc(md);

        // We don't care which pipeline we use to create the desc sets
		m_pipelines[0].AllocDescSets(m_modelContexts[MeshIndex].m_descSets);
		m_pipelines[0].UpdateDescriptorSets(md, m_modelContexts[MeshIndex].m_descSets,
												m_modelContexts[MeshIndex].m_uniformBuffersVS, 
												m_modelContexts[MeshIndex].m_uniformBuffersFS);
	}


	void RecordCommandBuffers()
	{
		// 1. Bake the baseline mesh draw calls (No conditional post-process paths inside here anymore!)
		for (int MeshIndex = 0; MeshIndex < (int)m_modelContexts.size(); MeshIndex++) {
			for (int LightMode = 0; LightMode < OgldevVK::NUM_LIGHTING_MODES; LightMode++) {
				// Note: RecordCommandBuffersInternal now only takes 3 parameters
				RecordCommandBuffersInternal(MeshIndex, LightMode, m_cmdBufs[MeshIndex][LightMode].BaseMeshDraw);
			}
		}

		InitToneMappingCommandBuffers();

		InitFallbackCopyCommandBuffers();

		InitTransitionCommandBuffers();

		InitToneMappingTransitionCommandBuffers();
	}


	void RecordCommandBuffersInternal(int MeshIndex, int LightingMode, std::vector<VkCommandBuffer>& CmdBufs)
	{
		bool IsFirstMesh = (MeshIndex == 0);
		VkFormat SwapChainFormat = m_vkCore.GetSwapChainFormat();
		VkFormat DepthFormat = m_vkCore.GetDepthFormat();
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_vkCore.GetSwapChainExtent().width;  // Match hardware width
		viewport.height = (float)m_vkCore.GetSwapChainExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D Scissor{};
		Scissor.offset = { 0, 0 };
		Scissor.extent.width = m_vkCore.GetSwapChainExtent().width;
		Scissor.extent.height = m_vkCore.GetSwapChainExtent().height;

		for (uint i = 0; i < CmdBufs.size(); i++) {
			VkCommandBuffer& CmdBuf = CmdBufs[i];

			OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			VkImageLayout SrcColorLayout = IsFirstMesh ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			OgldevVK::ImageMemBarrier2(CmdBuf, m_offlineImages[i].m_color.m_image, SwapChainFormat,
				SrcColorLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1, 0);

			// Depth handling (Remains untouched and safe from previous fixes)
			VkImageLayout SrcDepthLayout = IsFirstMesh ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			OgldevVK::ImageMemBarrier2(CmdBuf, m_offlineImages[i].m_depth.m_image, DepthFormat,
				SrcDepthLayout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, 1, 0);

			// Draw Geometry
			BeginRendering(CmdBuf, m_offlineImages[i].m_color.m_view, m_offlineImages[i].m_depth.m_view, IsFirstMesh);
			vkCmdSetViewport(CmdBuf, 0, 1, &viewport);
			vkCmdSetScissor(CmdBuf, 0, 1, &Scissor);
			m_pipelines[LightingMode].Bind(i, CmdBuf, m_modelContexts[MeshIndex].m_descSets[i], m_modelContexts[MeshIndex].m_baseTextureIndex);
			m_modelContexts[MeshIndex].m_pModel->RecordCommandBufferIndirect(CmdBuf);
			vkCmdEndRendering(CmdBuf);

			VkResult res = vkEndCommandBuffer(CmdBuf);
			CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
		}
	}


	// Fallback buffer used ONLY when Post-Processing is disabled to copy the frame over
	void InitFallbackCopyCommandBuffers()
	{
		m_fallbackCopyCmdBufs.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, m_fallbackCopyCmdBufs.data());
		VkFormat SwapChainFormat = m_vkCore.GetSwapChainFormat();

		VkExtent2D SwapchainExtent = m_vkCore.GetSwapChainExtent();

		for (int i = 0; i < m_numImages; i++) {
			VkCommandBuffer CmdBuf = m_fallbackCopyCmdBufs[i];
			VkImage SwapChainImage = m_vkCore.GetImage(i);
			VkImage OfflineImage = m_offlineImages[i].m_color.m_image;			

			OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			// 1. Transition Offline to Source, Swapchain to Destination
			OgldevVK::ImageMemBarrier2(CmdBuf, OfflineImage, SwapChainFormat,
				                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1, 0);
			OgldevVK::ImageMemBarrier2(CmdBuf, SwapChainImage, SwapChainFormat,
				                       VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1, 0);

			// 2. Use BLIT instead of COPY to automatically scale 1440 down to 1415 gracefully
			VkImageBlit BlitRegion{};
			BlitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			BlitRegion.srcOffsets[0] = { 0, 0, 0 };
			BlitRegion.srcOffsets[1] = { (i32)SwapchainExtent.width, (i32)SwapchainExtent.height, 1 }; 

			BlitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			BlitRegion.dstOffsets[0] = { 0, 0, 0 };
			BlitRegion.dstOffsets[1] = { (i32)SwapchainExtent.width, (i32)SwapchainExtent.height, 1 };
		
			vkCmdBlitImage(CmdBuf,
				OfflineImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				SwapChainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &BlitRegion, VK_FILTER_LINEAR); 

			OgldevVK::ImageMemBarrier2(CmdBuf, SwapChainImage, SwapChainFormat,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1, 0);

			vkEndCommandBuffer(CmdBuf);
		}
	}


	void InitToneMappingCommandBuffers()
	{
		m_toneMappingCmdBufs.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, m_toneMappingCmdBufs.data());
		VkFormat SwapChainFormat = m_vkCore.GetSwapChainFormat();

		for (int i = 0; i < m_numImages; i++) {
			VkCommandBuffer CmdBuf = m_toneMappingCmdBufs[i];
			VkImage SwapChainImage = m_vkCore.GetImage(i);
			VkImage OfflineImage = m_offlineImages[i].m_color.m_image;			

			OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			OgldevVK::ImageMemBarrier2(CmdBuf, OfflineImage, SwapChainFormat, 
									   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1, 0);

			OgldevVK::ImageMemBarrier2(CmdBuf, SwapChainImage, SwapChainFormat,
									   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1, 0);

			m_vkCore.BeginDynamicRenderingSwapChain(CmdBuf, i, NULL, NULL);
            m_toneMappingPipeline.Bind(CmdBuf, m_toneMappingDescSets[i]);
			m_toneMappingPipeline.RecordCommandBuffer(CmdBuf);
            vkCmdEndRendering(CmdBuf);
			
			VkResult res = vkEndCommandBuffer(CmdBuf);
			CHECK_VK_RESULT(res, "Failed to record compute post-process command buffer\n");
		}
	}


	void InitTransitionCommandBuffers()
	{
		m_transitionCmdBufs.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, m_transitionCmdBufs.data());
		VkFormat SwapChainFormat = m_vkCore.GetSwapChainFormat();

		for (int i = 0; i < m_numImages; i++) {
			VkCommandBuffer CmdBuf = m_transitionCmdBufs[i];
			VkImage CurrentImage = m_vkCore.GetImage(i);			

			OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			// FIXED: Accept GENERAL as the incoming layout, mapping it smoothly to PRESENT_SRC_KHR
			OgldevVK::ImageMemBarrier2(CmdBuf, CurrentImage, SwapChainFormat,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, 1, 0);

			VkResult res = vkEndCommandBuffer(CmdBuf);
			CHECK_VK_RESULT(res, "Failed to record transition command buffer\n");
		}
	}


	void InitToneMappingTransitionCommandBuffers()
	{
		m_toneMappingTransitionCmdBufs.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, m_toneMappingTransitionCmdBufs.data());
		VkFormat SwapChainFormat = m_vkCore.GetSwapChainFormat();

		for (int i = 0; i < m_numImages; i++) {
			VkCommandBuffer CmdBuf = m_toneMappingTransitionCmdBufs[i];
			VkImage CurrentImage = m_vkCore.GetImage(i);			

			OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			OgldevVK::ImageMemBarrier2(CmdBuf, CurrentImage, SwapChainFormat,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, 1, 0);

			VkResult res = vkEndCommandBuffer(CmdBuf);
			CHECK_VK_RESULT(res, "Failed to record compute transition command buffer\n");
		}
	}



	void BeginRendering(VkCommandBuffer CmdBuf, VkImageView ImageView, VkImageView DepthView, bool FirstCommandBuffer)
	{
        if (FirstCommandBuffer) {
			VkClearValue ClearColor = {
				.color = {1.0f, 0.0f, 0.0f, 1.0f},
			};

			VkClearValue DepthValue = {
				.depthStencil = {.depth = 1.0f, .stencil = 0 }
			};

			m_vkCore.BeginDynamicRendering(CmdBuf, ImageView, &ClearColor, DepthView, &DepthValue);

		} else {
			m_vkCore.BeginDynamicRendering(CmdBuf, ImageView, NULL, DepthView, NULL);
		}		
	}


	void UpdateGUI()
	{		
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::CollapsingHeader("Position")) {
			ImGui::DragFloat3("##Position", &m_position.x, 0.01f);
			ImGui::SameLine();
			if (ImGui::Button("Reset##Pos")) {
				m_position = glm::vec3(0.0f);
			}
		}

		if (ImGui::CollapsingHeader("Rotation")) {
			ImGui::DragFloat3("##Rotation", &m_rotation.x, 1.0f); // Degrees
			ImGui::SameLine();
			if (ImGui::Button("Reset##Rot")) {
				m_rotation = glm::vec3(0.0f);
			}
		}

		if (ImGui::CollapsingHeader("Scale")) {
			ImGui::DragFloat("##Scale", &m_scale, 0.001f, 0.001f, 2.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SameLine();
			if (ImGui::Button("Reset##Scale")) {
				m_scale = 1.0f;
			}
		}

        ImGui::Checkbox("Enable Tone Mapping", &m_enableToneMapping);

		static const char* lightingModeNames[] = { "Unlit", "Normals", "Ambient", "Full" };
		for (int i = 0; i < OgldevVK::NUM_LIGHTING_MODES; ++i) {
			ImGui::RadioButton(lightingModeNames[i], (int*)&m_lightingMode, i);
		}

		ImGui::gizmo3D("##Dir1", m_lightDir, 200.0f, imguiGizmo::modeDirection);

		ImGui::ColorEdit3("Light Color", (float*)&m_lightColor);

		ImGui::DragFloat("Ambient Light", &m_ambientLight, 0.01f, 0.0f, 2.0f, "%.2f");

		ImGui::DragFloat("Diffuse Light", &m_diffuseLight, 0.01f, 0.0f, 2.0f, "%.2f");

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::End();

		ImGui::Render();
	}


	void UpdateUniformBuffers(int MeshIndex, int ImageIndex)
	{		
		glm::mat4 Scale = m_scale * glm::scale(glm::mat4(1.0f), glm::vec3(Models[MeshIndex].Scale));

		glm::mat4 Translate = glm::translate(glm::mat4(1.0f), m_position + Models[MeshIndex].Pos);

		glm::mat4 World = Translate * Scale;

		glm::mat4 VP = m_pGameCamera->GetVPMatrix();

		glm::mat4 WVP = VP * World;

        glm::vec4 AmbientLight = glm::vec4(m_lightColor.x, m_lightColor.y, m_lightColor.z, m_ambientLight);
        glm::vec4 DiffuseLight = glm::vec4(m_lightColor.x, m_lightColor.y, m_lightColor.z, m_diffuseLight);
		glm::vec3 LightDirection = glm::vec3(-m_lightDir.x, -m_lightDir.y, -m_lightDir.z);
		//printf("Light dir: %f %f %f\n", LightDirection.x, LightDirection.y, LightDirection.z);
        
		OgldevVK::LightingProgram::UpdateUniformBuffers(m_device, 
														WVP, 
														World, 
														m_modelContexts[MeshIndex].m_pModel->GetTransformations(), 
														AmbientLight,
														DiffuseLight,
														LightDirection, 
														m_modelContexts[MeshIndex].m_uniformBuffersVS[ImageIndex],
														m_modelContexts[MeshIndex].m_uniformBuffersFS[ImageIndex]);
	}

	GLFWwindow* m_pWindow = NULL;
	OgldevVK::VulkanCore m_vkCore;
	VkDescriptorPool m_descPool = VK_NULL_HANDLE;
	OgldevVK::VulkanQueue* m_pQueue = NULL;
	VkDevice m_device = NULL;
    std::vector<OfflineImage> m_offlineImages;
	int m_numImages = 0;
	struct MeshCmdBufs {
		std::vector<VkCommandBuffer> BaseMeshDraw; // Size: m_numImages
	};
    std::vector<std::vector<MeshCmdBufs>> m_cmdBufs;	// outer dim: meshes, inner dim: lighting modes
	std::vector<VkCommandBuffer> m_transitionCmdBufs;
	std::vector<VkCommandBuffer> m_toneMappingTransitionCmdBufs;
	std::vector<VkCommandBuffer> m_toneMappingCmdBufs;
	std::vector<VkCommandBuffer> m_fallbackCopyCmdBufs;
	VkShaderModule m_vs = VK_NULL_HANDLE;
	VkShaderModule m_fs = VK_NULL_HANDLE;
	OgldevVK::LightingProgram m_pipelines[OgldevVK::NUM_LIGHTING_MODES];
    OgldevVK::ToneMappingProgram m_toneMappingPipeline;
	std::vector<ModelContext> m_modelContexts;
	std::vector<VkDescriptorSet> m_toneMappingDescSets;
	GLMCameraFirstPerson* m_pGameCamera = NULL;
	OgldevVK::ImGUIRenderer m_imGUIRenderer;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	OgldevVK::BigTextureArray m_bigTextureArray;

	// GUI state
	bool m_showGui = false;
    bool m_enableToneMapping = true;
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);
	float m_scale = 0.1f;
	OgldevVK::LIGHTING_MODE m_lightingMode = OgldevVK::LIGHTING_MODE_FULL;
	vec3 m_lightDir = vec3(0.0f, 0.14f, 1.0f);
    float m_ambientLight = 0.1f;
    float m_diffuseLight = 1.0f;
    vec3 m_lightColor = vec3(1.0f, 1.0f, 1.0f);
};


int main(int argc, char* argv[])
{
	VulkanApp App(WINDOW_WIDTH, WINDOW_HEIGHT);

	App.Init(APP_NAME);

	App.Execute();

	return 0;
}