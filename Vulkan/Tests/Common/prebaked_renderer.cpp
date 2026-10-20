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
*/

#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "prebaked_renderer.h"
#include "ogldev_vulkan_wrapper.h"
#include "ogldev_vulkan_shader.h"
//#include "Int/model_desc.h"

#define MAX_TEXTURES 4096

#define BIG_TEXTURE_ARRAY_BINDING 0

#define NAME_OF_THE_GAME "Pong"


VkFormat OffscreenColorFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

PreBakedRenderer::PreBakedRenderer(int WindowWidth, int WindowHeight, 
								   const std::string& AppName, GameCallbacks* pGameCallbacks)
{
	m_windowWidth = WindowWidth;
	m_windowHeight = WindowHeight;
	m_appName = AppName;
	m_pGameCallbacks = pGameCallbacks;
    m_lights.resize(MAX_NUM_LIGHTS);
}


PreBakedRenderer::~PreBakedRenderer()
{
	if (m_vkCore.GetDevice() != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(m_vkCore.GetDevice());
	}

	m_imGUIRenderer.Destroy();

    for (MeshCmdBufs& v : m_cmdBufs) {
		m_vkCore.FreeCommandBuffers((u32)v.BaseMeshDraw.size(), v.BaseMeshDraw.data());
	}
        
	m_modelContext.Destroy(m_device);

	vkDestroyShaderModule(m_device, m_vs, NULL);
	vkDestroyShaderModule(m_device, m_fs, NULL);

	for (OgldevVK::LightingProgram& p : m_pipelines) {
		p.Destroy();
	}
        
    for (int i = 0; i < (int)m_offscreenImages.size(); i++) {
        m_offscreenImages[i].Destroy(m_device);
    }

    m_bigTextureArray.Destroy();		

	vkDestroyDescriptorPool(m_device, m_descPool, NULL);

	glfwTerminate();
}


void PreBakedRenderer::Init(const std::string& AssetPath)
{
	m_pWindow = OgldevVK::glfw_vulkan_init(m_windowWidth, m_windowHeight, m_appName.c_str());

    m_vkCore.Init(m_appName.c_str(), m_pWindow, (OgldevVK::InitFlags)(OgldevVK::OGLDEV_VK_INIT_COMPUTE_ENABLED));
	m_device = m_vkCore.GetDevice();
	m_numImages = m_vkCore.GetNumImages();
	m_pQueue = m_vkCore.GetQueue();
	CreateShaders();
	CreateDescriptorPool();
	InitBigTextureArray();
	CreateOffscreenImages();
	CreatePipelines();
	CreateMesh(AssetPath);
	CreateCommandBuffers();
	RecordCommandBuffers();
	InitCameraFromModel();
	// The object is ready to receive callbacks
	OgldevVK::glfw_vulkan_set_callbacks(m_pWindow, this);
	m_imGUIRenderer.Init(&m_vkCore);
}


void PreBakedRenderer::RenderScene()
{
	u32 ImageIndex = m_pQueue->AcquireNextImage();

	UpdateUniformBuffers(ImageIndex);

	std::vector<VkCommandBuffer> SubmissionCmdBufs;

	SubmissionCmdBufs.push_back(m_cmdBufs[m_lightingMode].BaseMeshDraw[ImageIndex]);

	SubmissionCmdBufs.push_back(m_fallbackCopyCmdBufs[ImageIndex]);

	if (m_showGui) {
		UpdateGUI();
		VkCommandBuffer ImGUICmdBuf = m_imGUIRenderer.PrepareCommandBuffer(ImageIndex);
		SubmissionCmdBufs.push_back(ImGUICmdBuf);
	} else {
		SubmissionCmdBufs.push_back(m_swapChainColorToPresentCmdBufs[ImageIndex]);
	}

	m_pQueue->SubmitAsync(SubmissionCmdBufs);
	m_pQueue->Present(ImageIndex);
}

	
void PreBakedRenderer::Key(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods)
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

        if (!Handled && m_pGameCallbacks) {
            m_pGameCallbacks->OnKey(Key, Scancode, Action, Mods);
        }
	}
}
	
	
void PreBakedRenderer::MouseMove(GLFWwindow* pWindow, double x, double y)
{
	m_pGameCamera->SetMousePos((float)x, (float)y);
}


void PreBakedRenderer::MouseButton(GLFWwindow* pWindow, int Button, int Action, int Mods)
{
	if (m_showGui && IsMouseControlledByImGUI()) {
		return;
	}
		
	m_pGameCamera->HandleMouseButton(Button, Action, Mods);
}
	
	
void PreBakedRenderer::Execute()
{
	float CurTime = (float)glfwGetTime();

	int Frames = 0;
	float FPSTime = 0.0f;
	while (!glfwWindowShouldClose(m_pWindow)) {
		double Time = glfwGetTime();
		float dt = (float)(Time - CurTime);
        if (m_pGameCallbacks) {
            m_pGameCallbacks->UpdateGameState(dt);
        }
		m_pGameCamera->Update(dt);
		RenderScene();
		CurTime = (float)Time;
		glfwPollEvents();

		Frames++;
		FPSTime += dt;

		if (FPSTime >= 1.0f) {
			char Title[256];
			snprintf(Title, sizeof(Title), "%s : FPS %d\n", m_appName.c_str(), Frames);
			glfwSetWindowTitle(m_pWindow, Title);
			FPSTime = 0.0f;
			Frames = 0;
		}
	}
}


void PreBakedRenderer::InitCameraFromModel()
{
    if (m_modelContext.m_pModel->GetCameras().size() == 0) {
        OGLDEV_ERROR("Model has no cameras.");
        assert(0);
    }

    m_pGameCamera = (GLMCameraFirstPerson*)&m_modelContext.m_pModel->GetCameras()[0];
}


void PreBakedRenderer::CreateCommandBuffers()
{		
	m_cmdBufs.resize(OgldevVK::NUM_LIGHTING_MODES);

	for (MeshCmdBufs& v : m_cmdBufs) {
		v.BaseMeshDraw.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, v.BaseMeshDraw.data());
	}

	printf("Created command buffers\n");
}


void PreBakedRenderer::CreateDescriptorPool()
{
	u32 TextureCount = MAX_TEXTURES * 4;
	u32 UniformBufferCount = 50;
	u32 StorageBufferCount = 50;
	u32 StorageImageCount = m_numImages;
	u32 MaxSets = (m_numImages * (OgldevVK::NUM_LIGHTING_MODES + 1)) + m_numImages; // +1 for the global texture array descriptor set

	m_descPool = m_vkCore.CreateDescPool(TextureCount, UniformBufferCount, StorageBufferCount, StorageImageCount, MaxSets);
}


void PreBakedRenderer::InitBigTextureArray()
{
    m_bigTextureArray.Init(m_vkCore.GetDevice(), m_descPool, MAX_TEXTURES, BIG_TEXTURE_ARRAY_BINDING);
}


void PreBakedRenderer::CreateOffscreenImages()
{
    m_offscreenImages.resize(m_numImages);

	VkExtent2D SwapChainExtent = m_vkCore.GetSwapChainExtent();
    VkFormat DepthFormat = m_vkCore.GetDepthFormat();
	VkImageUsageFlags ColorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
								  	VK_IMAGE_USAGE_SAMPLED_BIT |
									VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	VkImageUsageFlags DepthUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    for (int i = 0; i < (int)m_offscreenImages.size(); i++) {
        m_vkCore.CreateTexture(m_offscreenImages[i].m_color, SwapChainExtent.width, SwapChainExtent.height, 
								ColorUsage, OffscreenColorFormat, false);
			
        m_vkCore.CreateTexture(m_offscreenImages[i].m_depth, SwapChainExtent.width, SwapChainExtent.height, 
								DepthUsage, DepthFormat, false);

    }
}


void PreBakedRenderer::CreateMesh(const std::string& AssetPath)
{
    m_modelContext.m_pModel = new OgldevVK::VkModel();

    // We don't really need a vector here, but we need to pass a vector to the CreateTextureArray function
	std::vector<OgldevVK::ModelDesc> ModelDescs(1);
		
	m_modelContext.m_pModel->Init(&m_vkCore, true, false);
	m_modelContext.m_pModel->LoadAssimpModel(AssetPath);
    CreateUniformBuffers(0);
    CreateDescriptorSets(0, ModelDescs[0]);

	m_bigTextureArray.CreateTextureArray(ModelDescs);

    if (m_modelContext.m_pModel->GetDirLights().size() > 0) {
        Vector3f LightDir = m_modelContext.m_pModel->GetDirLights()[0].WorldDirection;
        // m_lightDir points toward the light (it is negated in UpdateUniformBuffers)
        m_lightDir = vec3(LightDir.x, LightDir.y, LightDir.z);
		m_diffuseIntensity = m_modelContext.m_pModel->GetDirLights()[0].DiffuseIntensity;
        m_lightColor = vec3(m_modelContext.m_pModel->GetDirLights()[0].Color.x,
            m_modelContext.m_pModel->GetDirLights()[0].Color.y,
            m_modelContext.m_pModel->GetDirLights()[0].Color.z);

		m_lights[0] = {
			.Color = glm::vec3(m_lightColor.x, m_lightColor.y, m_lightColor.z),
            .LightType = LIGHT_TYPE_DIR,
            .Direction = glm::vec3(m_lightDir.x, m_lightDir.y, m_lightDir.z),
			.AmbientIntensity = m_ambientIntensity,
			.WorldPos = glm::vec3(0.0f),
			.DiffuseIntensity = m_diffuseIntensity
		};
    }
}


void PreBakedRenderer::CreateShaders()
{
	m_vs = OgldevVK::CreateShaderModuleFromText(m_device, "../Common/lighting.vert");

	m_fs = OgldevVK::CreateShaderModuleFromText(m_device, "../Common/lighting.frag");
}


void PreBakedRenderer::CreatePipelines()
{
	for (int i = 0; i < OgldevVK::NUM_LIGHTING_MODES; i++) {
		m_pipelines[i].Init(m_vkCore, OffscreenColorFormat, m_descPool, m_bigTextureArray.GetDescSetLayout(), 
				            m_bigTextureArray.GetDescSet(), m_vs, m_fs, (OgldevVK::LIGHTING_MODE)i);
	}
}


void PreBakedRenderer::CreateUniformBuffers(int MeshIndex)
{		
    // VS uniform buffers (actually using SSBOs)
    size_t NumMeshes = m_modelContext.m_pModel->GetNumMeshes();
	size_t UniformBufferSizeVS = OgldevVK::LightingProgram::GetUniformBufferSizeVS(NumMeshes);

    m_modelContext.m_uniformBuffersVS.resize(m_numImages);

    for (int i = 0; i < m_numImages; i++) {
		m_modelContext.m_uniformBuffersVS[i] = m_vkCore.CreateSSBO(UniformBufferSizeVS);
    }

    // FS uniform buffers
	size_t UniformBufferSizeFS = OgldevVK::LightingProgram::GetUniformBufferSizeFS();
	m_modelContext.m_uniformBuffersFS = m_vkCore.CreateUniformBuffers(UniformBufferSizeFS);
}


void PreBakedRenderer::CreateDescriptorSets(int MeshIndex, OgldevVK::ModelDesc& md)
{		
	m_modelContext.m_pModel->UpdateModelDesc(md);

    // We don't care which pipeline we use to create the desc sets
	m_pipelines[0].AllocDescSets(m_modelContext.m_descSets);
	m_pipelines[0].UpdateDescriptorSets(md, m_modelContext.m_descSets,
											m_modelContext.m_uniformBuffersVS, 
											m_modelContext.m_uniformBuffersFS);
}


void PreBakedRenderer::RecordCommandBuffers()
{
	// 1. Bake the baseline mesh draw calls (No conditional post-process paths inside here anymore!)
	for (int LightMode = 0; LightMode < OgldevVK::NUM_LIGHTING_MODES; LightMode++) {
		// Note: RecordCommandBuffersInternal now only takes 3 parameters
		RecordCommandBuffersInternal(LightMode, m_cmdBufs[LightMode].BaseMeshDraw);
	}

	RecordFallbackCopyCommandBuffers();

	RecordSwapChainColorToPresentCommandBuffers();
}


void PreBakedRenderer::RecordCommandBuffersInternal(int LightingMode, std::vector<VkCommandBuffer>& CmdBufs)
{
	VkFormat DepthFormat = m_vkCore.GetDepthFormat();

	for (uint i = 0; i < CmdBufs.size(); i++) {
		VkCommandBuffer& CmdBuf = CmdBufs[i];
		OgldevVK::VulkanTexture& OffscreenColorImage = m_offscreenImages[i].m_color;
		OgldevVK::VulkanTexture& OffscreenDepthImage = m_offscreenImages[i].m_depth;

		OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		VkImageLayout SrcColorLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        OffscreenColorImage.TransitionLayout(CmdBuf, SrcColorLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		// Depth handling (Remains untouched and safe from previous fixes)
		VkImageLayout SrcDepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		OffscreenDepthImage.TransitionLayout(CmdBuf, SrcDepthLayout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		// Draw Geometry
		BeginRendering(CmdBuf, OffscreenColorImage.m_view, OffscreenDepthImage.m_view);
		m_pipelines[LightingMode].Bind(i, CmdBuf, m_modelContext.m_descSets[i], m_modelContext.m_baseTextureIndex);
		m_modelContext.m_pModel->RecordCommandBufferIndirect(CmdBuf);
		vkCmdEndRendering(CmdBuf);

		VkResult res = vkEndCommandBuffer(CmdBuf);
		CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
	}
}


void PreBakedRenderer::RecordFallbackCopyCommandBuffers()
{
	m_fallbackCopyCmdBufs.resize(m_numImages);
	m_vkCore.CreateCommandBuffers(m_numImages, m_fallbackCopyCmdBufs.data());
	VkExtent2D SwapChainExtent = m_vkCore.GetSwapChainExtent();

	for (int i = 0; i < m_numImages; i++) {
		VkCommandBuffer CmdBuf = m_fallbackCopyCmdBufs[i];
        OgldevVK::VulkanBaseImage& SwapChainImage = m_vkCore.GetSwapChainImage(i);
		OgldevVK::VulkanTexture& OffscreenImage = m_offscreenImages[i].m_color;

		OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        OffscreenImage.TransitionLayout(CmdBuf, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
												VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        SwapChainImage.TransitionLayout(CmdBuf, VK_IMAGE_LAYOUT_UNDEFINED, 
												VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageBlit BlitRegion{};
		BlitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		BlitRegion.srcOffsets[0] = { 0, 0, 0 };
		BlitRegion.srcOffsets[1] = { (i32)SwapChainExtent.width, (i32)SwapChainExtent.height, 1 }; 

		BlitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		BlitRegion.dstOffsets[0] = { 0, 0, 0 };
		BlitRegion.dstOffsets[1] = { (i32)SwapChainExtent.width, (i32)SwapChainExtent.height, 1 };
		
		vkCmdBlitImage(CmdBuf, OffscreenImage.m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				        SwapChainImage.m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				        1, &BlitRegion, VK_FILTER_LINEAR); 

        SwapChainImage.TransitionLayout(CmdBuf, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		vkEndCommandBuffer(CmdBuf);
	}
}


void PreBakedRenderer::RecordSwapChainColorToPresentCommandBuffers()
{
	m_swapChainColorToPresentCmdBufs.resize(m_numImages);
	m_vkCore.CreateCommandBuffers(m_numImages, m_swapChainColorToPresentCmdBufs.data());

	for (int i = 0; i < m_numImages; i++) {
		VkCommandBuffer CmdBuf = m_swapChainColorToPresentCmdBufs[i];
		OgldevVK::VulkanBaseImage& SwapChainImage = m_vkCore.GetSwapChainImage(i);			

		OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        SwapChainImage.TransitionLayout(CmdBuf, 
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
                                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		VkResult res = vkEndCommandBuffer(CmdBuf);
		CHECK_VK_RESULT(res, "Failed to record transition command buffer\n");
	}
}


void PreBakedRenderer::BeginRendering(VkCommandBuffer CmdBuf, VkImageView ImageView, VkImageView DepthView)
{
	VkClearValue ClearColor = {
		.color = {1.0f, 0.0f, 0.0f, 1.0f},
	};

	VkClearValue DepthValue = {
		.depthStencil = {.depth = 1.0f, .stencil = 0 }
	};

	m_vkCore.BeginDynamicRendering(CmdBuf, ImageView, &ClearColor, DepthView, &DepthValue);
}


void PreBakedRenderer::UpdateGUI()
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

	static const char* lightingModeNames[] = { "Unlit", "Normals", "Ambient", "Full" };
	for (int i = 0; i < OgldevVK::NUM_LIGHTING_MODES; ++i) {
		ImGui::RadioButton(lightingModeNames[i], (int*)&m_lightingMode, i);
	}

	ImGui::gizmo3D("##Dir1", m_lightDir, 200.0f, imguiGizmo::modeDirection);

	ImGui::ColorEdit3("Light Color", (float*)&m_lightColor);

	ImGui::DragFloat("Ambient Intensity", &m_ambientIntensity, 0.01f, 0.0f, 1.0f, "%.2f");

	ImGui::DragFloat("Diffuse Intensity", &m_diffuseIntensity, 0.01f, 0.0f, 5.0f, "%.2f");

	m_lights[0].Color = glm::vec3(m_lightColor.x, m_lightColor.y, m_lightColor.z);
	m_lights[0].Direction = glm::vec3(m_lightDir.x, m_lightDir.y, m_lightDir.z);
	m_lights[0].AmbientIntensity = m_ambientIntensity;
	m_lights[0].DiffuseIntensity = m_diffuseIntensity;

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

	ImGui::End();

	ImGui::Render();
}


void PreBakedRenderer::UpdateUniformBuffers(int ImageIndex)
{		
    glm::mat4 World = glm::mat4(1.0f);

	glm::mat4 VP = m_pGameCamera->GetVPMatrix();

	glm::mat4 WVP = VP * World;
      
	OgldevVK::LightingProgram::UpdateUniformBuffers(m_device, 
													m_pGameCamera->GetPosition(),
													WVP, 
													World, 
													m_modelContext.m_pModel->GetTransformations(), 
												    m_lights,
													m_modelContext.m_uniformBuffersVS[ImageIndex],
													m_modelContext.m_uniformBuffersFS[ImageIndex]);
}