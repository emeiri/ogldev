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
#include "prebaked_renderer.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define MAX_TEXTURES 4096

#define BIG_TEXTURE_ARRAY_BINDING 0

#define NAME_OF_THE_GAME "Pong"


VkFormat OffscreenColorFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

PreBakedRenderer::PreBakedRenderer(int WindowWidth, int WindowHeight, const std::string& AppName)
{
	m_windowWidth = WindowWidth;
	m_windowHeight = WindowHeight;
	m_appName = AppName;
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
        
    m_toneMappingPipeline.Destroy();

    for (int i = 0; i < (int)m_offscreenImages.size(); i++) {
        m_offscreenImages[i].Destroy(m_device);
    }

    m_bigTextureArray.Destroy();		

	vkDestroyDescriptorPool(m_device, m_descPool, NULL);

	glfwTerminate();
}


void PreBakedRenderer::Init(const std::string& AssetPath)
{
	m_pWindow = OgldevVK::glfw_vulkan_init(WINDOW_WIDTH, WINDOW_HEIGHT, m_appName.c_str());

    m_vkCore.Init(m_appName.c_str(), m_pWindow, (OgldevVK::InitFlags)(OgldevVK::OGLDEV_VK_INIT_COMPUTE_ENABLED));
	m_device = m_vkCore.GetDevice();
	m_numImages = m_vkCore.GetNumImages();
	m_pQueue = m_vkCore.GetQueue();
	CreateShaders();
	CreateDescriptorPool();
	InitBigTextureArray();
	CreateOffscreenImages();
	CreatePipelines();
	CreateMeshe(AssetPath);
	CreateCommandBuffers();
	RecordCommandBuffers();
	InitCameraFromModel();
	//DefaultCreateCameraPers();
	// The object is ready to receive callbacks
	OgldevVK::glfw_vulkan_set_callbacks(m_pWindow, this);
	m_imGUIRenderer.Init(&m_vkCore);
}


void PreBakedRenderer::RenderScene()
{
	u32 ImageIndex = m_pQueue->AcquireNextImage();

	UpdateUniformBuffers(ImageIndex);

	std::vector<VkCommandBuffer> SubmissionCmdBufs;

	// 1. Geometry Pass
	SubmissionCmdBufs.push_back(m_cmdBufs[m_lightingMode].BaseMeshDraw[ImageIndex]);

	// 2. Resolve Post-Processing and Presentation Chains
	if (m_enableToneMapping) {
		SubmissionCmdBufs.push_back(m_toneMappingCmdBufs[ImageIndex]);
	} else {
		// Fallback Blit Path
		SubmissionCmdBufs.push_back(m_fallbackCopyCmdBufs[ImageIndex]);
    }

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
			snprintf(Title, sizeof(Title), "%s : FPS %d\n", m_appName.c_str(), Frames);
			glfwSetWindowTitle(m_pWindow, Title);
			FPSTime = 0.0f;
			Frames = 0;
		}
	}
}


void PreBakedRenderer::InitCameraFromModel()
{
	glm::vec3 Pos = m_modelContext.m_pModel->GetCameras()[0].GetPosition();
    //  glm::vec3 Target = mctx.Pos;
    glm::vec3 Up(0.0, 1.0f, 0.0f);
    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 1500.0f;
    PersProjInfo persProjInfo = { FOV, (float)m_windowWidth, (float)m_windowHeight,
                                    zNear, zFar };
    //  m_pGameCamera = new GLMCameraFirstPerson(Pos, Target, Up, persProjInfo);
    m_pGameCamera = (GLMCameraFirstPerson*) &m_modelContext.m_pModel->GetCameras()[0];
    //  m_pGameCamera->m_maxSpeed = 1.5f;
}


void PreBakedRenderer::DefaultCreateCameraPers()
{
	float FOV = 45.0f;
	float zNear = 0.1f;
	float zFar = 1500.0f;

	DefaultCreateCameraPers(FOV, zNear, zFar);
}


void PreBakedRenderer::DefaultCreateCameraPers(float FOV, float zNear, float zFar)
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


void PreBakedRenderer::CreateMeshe(const std::string& AssetPath)
{
    m_modelContext.m_pModel = new OgldevVK::VkModel();

    // We don't really need a vector here, but we need to pass a vector to the CreateTextureArray function
	std::vector<OgldevVK::ModelDesc> ModelDescs(1);
		
	m_modelContext.m_pModel->Init(&m_vkCore, true, false);
	m_modelContext.m_pModel->LoadAssimpModel(AssetPath);
    CreateUniformBuffers(0);
    CreateDescriptorSets(0, ModelDescs[0]);

	m_toneMappingPipeline.AllocDescSets(m_toneMappingDescSets);
    m_toneMappingPipeline.UpdateDescriptorSets(m_toneMappingDescSets, m_offscreenImages);
		
	m_bigTextureArray.CreateTextureArray(ModelDescs);
}


void PreBakedRenderer::CreateShaders()
{
	m_vs = OgldevVK::CreateShaderModuleFromText(m_device, "test.vert");

	m_fs = OgldevVK::CreateShaderModuleFromText(m_device, "test.frag");
}


void PreBakedRenderer::CreatePipelines()
{
	for (int i = 0; i < OgldevVK::NUM_LIGHTING_MODES; i++) {
		m_pipelines[i].Init(m_vkCore, OffscreenColorFormat, m_descPool, m_bigTextureArray.GetDescSetLayout(), 
				            m_bigTextureArray.GetDescSet(), m_vs, m_fs, (OgldevVK::LIGHTING_MODE)i);
	}

    m_toneMappingPipeline.Init(m_vkCore, m_descPool);
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

	RecordToneMappingCommandBuffers();

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


void PreBakedRenderer::RecordToneMappingCommandBuffers()
{
	m_toneMappingCmdBufs.resize(m_numImages);
	m_vkCore.CreateCommandBuffers(m_numImages, m_toneMappingCmdBufs.data());

	for (int i = 0; i < m_numImages; i++) {
		VkCommandBuffer CmdBuf = m_toneMappingCmdBufs[i];
		OgldevVK::VulkanBaseImage& SwapChainImage = m_vkCore.GetSwapChainImage(i);
		OgldevVK::VulkanTexture& OffscreenColorImage = m_offscreenImages[i].m_color;

		OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		OffscreenColorImage.TransitionLayout(CmdBuf,
											VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
											VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        SwapChainImage.TransitionLayout(CmdBuf, 
										VK_IMAGE_LAYOUT_UNDEFINED, 
										VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		m_vkCore.BeginDynamicRenderingSwapChain(CmdBuf, i, NULL, NULL);
        m_toneMappingPipeline.Bind(CmdBuf, m_toneMappingDescSets[i]);
		m_toneMappingPipeline.RecordCommandBuffer(CmdBuf);

        vkCmdEndRendering(CmdBuf);
			
		VkResult res = vkEndCommandBuffer(CmdBuf);
		CHECK_VK_RESULT(res, "Failed to record compute post-process command buffer\n");
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

	ImGui::DragFloat("Ambient Light", &m_ambientLight, 0.01f, 0.0f, 1.0f, "%.2f");

	ImGui::DragFloat("Diffuse Light", &m_diffuseLight, 0.01f, 0.0f, 5.0f, "%.2f");

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

	ImGui::End();

	ImGui::Render();
}


void PreBakedRenderer::UpdateUniformBuffers(int ImageIndex)
{		
	glm::mat4 Translate = glm::translate(glm::mat4(1.0f), m_position);

	glm::mat4 World = Translate;

	glm::mat4 VP = m_pGameCamera->GetVPMatrix();

	glm::mat4 WVP = VP * World;

    glm::vec4 AmbientLight = glm::vec4(m_lightColor.x, m_lightColor.y, m_lightColor.z, m_ambientLight);
    glm::vec4 DiffuseLight = glm::vec4(m_lightColor.x, m_lightColor.y, m_lightColor.z, m_diffuseLight);
	glm::vec3 LightDirection = glm::vec3(-m_lightDir.x, -m_lightDir.y, -m_lightDir.z);
	//printf("Light dir: %f %f %f\n", LightDirection.x, LightDirection.y, LightDirection.z);
        
	OgldevVK::LightingProgram::UpdateUniformBuffers(m_device, 
													WVP, 
													World, 
													m_modelContext.m_pModel->GetTransformations(), 
													AmbientLight,
													DiffuseLight,
													LightDirection, 
													m_modelContext.m_uniformBuffersVS[ImageIndex],
													m_modelContext.m_uniformBuffersFS[ImageIndex]);
}