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
		Tutorial #30: Push Constants
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
#include "imGuIZMOquat.h"			// optional

#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_wrapper.h"
#include "ogldev_vulkan_shader.h"
#include "ogldev_vulkan_glfw.h"
#include "ogldev_vulkan_model.h"
#include "ogldev_vulkan_skybox.h"
#include "ogldev_glm_camera.h"
#include "ogldev_vulkan_imgui.h"
#include "Int/model_desc.h"
#include "lighting_program.h"
#include "big_texture_array.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define MAX_TEXTURES 4096

#define BIG_TEXTURE_ARRAY_BINDING 0

#define APP_NAME "Tutorial 30"

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
	{ "../../Content/crytek_sponza/sponza.obj", glm::vec3(0.0f), 0.01f },
	{ "../../Content/vintage_cabinet_01/vintage_cabinet_01_4k.gltf", glm::vec3(-8.0f, 0.0f, -1.5f), 1.0f},
	{ "../../Content/box.obj", glm::vec3(2.0f, 0.5f, -1.5f), 0.25f},
	{ "../../Content/antique_ceramic_vase_01_4k.blend/antique_ceramic_vase_01_4k.obj", glm::vec3(-4.0f, 0.0f, -1.5f), 2.0f},
	{ "../../Content/Stanford/stanford_dragon_pbr/scene.gltf", glm::vec3(0.0f, 0.0f, -1.5f), 0.02f }
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
        for (int MeshIndex = 0; MeshIndex < m_modelContexts.size(); MeshIndex++) {
			for (CommandBuffersVecs& v : m_cmdBufs[MeshIndex]) {
				m_vkCore.FreeCommandBuffers((u32)v.WithGUI.size(), v.WithGUI.data());
				m_vkCore.FreeCommandBuffers((u32)v.WithoutGUI.size(), v.WithoutGUI.data());
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

        m_bigTextureArray.Destroy();		

		vkDestroyDescriptorPool(m_device, m_descPool, NULL);			

		//m_skybox.Destroy();

		m_imGUIRenderer.Destroy();
	}


	void Init(const char* pAppName)
	{
		m_pWindow = OgldevVK::glfw_vulkan_init(WINDOW_WIDTH, WINDOW_HEIGHT, pAppName);

		m_vkCore.Init(pAppName, m_pWindow, true, false);
		m_device = m_vkCore.GetDevice();
		m_numImages = m_vkCore.GetNumImages();
		m_pQueue = m_vkCore.GetQueue();
		CreateShaders();
		CreateDescriptorPool();
		InitBigTextureArray();
		CreatePipeline();
		CreateMeshes();
		//m_skybox.Init(&m_vkCore, "../../Content/textures/evening_road_01_puresky_8k_2.jpg");		
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

		if (m_showGui) {		
			UpdateGUI();

			VkCommandBuffer ImGUICmdBuf = m_imGUIRenderer.PrepareCommandBuffer(ImageIndex);

			VkCommandBuffer CmdBufs[] = { m_cmdBufs[0][m_lightingMode].WithGUI[ImageIndex], ImGUICmdBuf};

			m_pQueue->SubmitAsync(&CmdBufs[0], 2);
		} else {
            std::vector<VkCommandBuffer> CmdBufs(m_modelContexts.size());

            for (int MeshIndex = 0; MeshIndex < m_modelContexts.size(); MeshIndex++) {
                CmdBufs[MeshIndex] = m_cmdBufs[MeshIndex][m_lightingMode].WithoutGUI[ImageIndex];
            }			

			m_pQueue->SubmitAsync(CmdBufs);
		}

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
		if (!IsMouseControlledByImGUI()) {
			m_pGameCamera->HandleMouseButton(Button, Action, Mods);
		}		
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

		glfwTerminate();
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
		glm::vec3 Target(-1.0f, -0.025f, 0.05f);
		glm::vec3 Up(0.0, 1.0f, 0.0f);

		m_pGameCamera = new GLMCameraFirstPerson(Pos, Target, Up, persProjInfo);
	}


	void CreateCommandBuffers()
	{		
        m_cmdBufs.resize(m_modelContexts.size());

        for (int i = 0; i < m_modelContexts.size(); i++) {
			m_cmdBufs[i].resize(OgldevVK::NUM_LIGHTING_MODES);

			for (CommandBuffersVecs& v : m_cmdBufs[i]) {
				v.WithGUI.resize(m_numImages);
				m_vkCore.CreateCommandBuffers(m_numImages, v.WithGUI.data());

				v.WithoutGUI.resize(m_numImages);
				m_vkCore.CreateCommandBuffers(m_numImages, v.WithoutGUI.data());
			}
        }

		printf("Created command buffers\n");
	}


	void CreateDescriptorPool()
	{
		u32 TextureCount = MAX_TEXTURES * 4;
		u32 UniformBufferCount = 50;
		u32 StorageBufferCount = 50;
        u32 MaxSets = m_numImages * (u32)Models.size() * (OgldevVK::NUM_LIGHTING_MODES + 1);	// +1 for the global texture array descriptor set

		m_descPool = m_vkCore.CreateDescPool(TextureCount, UniformBufferCount, StorageBufferCount, MaxSets);
	}


	void InitBigTextureArray()
	{
        m_bigTextureArray.Init(m_vkCore.GetDevice(), m_descPool, m_vkCore.GetNumImages(), MAX_TEXTURES, BIG_TEXTURE_ARRAY_BINDING);
	}


	void CreateMeshes()
	{
        m_modelContexts.resize(Models.size());

        std::vector<OgldevVK::ModelDesc> ModelDescs(m_modelContexts.size());
		
		for (int i = 0; i < (int)m_modelContexts.size(); i++) {
			m_modelContexts[i].m_pModel = new OgldevVK::VkModel(true);
			m_modelContexts[i].m_pModel->Init(&m_vkCore);
			m_modelContexts[i].m_pModel->LoadAssimpModel(Models[i].Path);
            CreateUniformBuffers(i);
            CreateDescriptorSets(i, ModelDescs[i]);
		}

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
				&m_bigTextureArray.GetDescSets(), m_vs, m_fs, (OgldevVK::LIGHTING_MODE)i);
		}
	}


	void CreateUniformBuffers(int MeshIndex)
	{		
        size_t UniformBufferSizeVS = OgldevVK::LightingProgram::GetUniformBufferSizeVS(m_modelContexts[MeshIndex].m_pModel->GetNumMeshes());
		m_modelContexts[MeshIndex].m_uniformBuffersVS = m_vkCore.CreateUniformBuffers(UniformBufferSizeVS);

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
		for (int MeshIndex = 0; MeshIndex < (int)m_modelContexts.size(); MeshIndex++) {
			for (int LightMode = 0; LightMode < OgldevVK::NUM_LIGHTING_MODES; LightMode++) {
				RecordCommandBuffersInternal(MeshIndex, LightMode, true, m_cmdBufs[MeshIndex][LightMode].WithoutGUI);

				RecordCommandBuffersInternal(MeshIndex, LightMode, false, m_cmdBufs[MeshIndex][LightMode].WithGUI);
			}
		}
	}


	void RecordCommandBuffersInternal(int MeshIndex, int LightingMode, bool WithSecondBarrier, std::vector<VkCommandBuffer>& CmdBufs)
	{
		bool FirstCommandBuffer = (MeshIndex == 0);

		for (uint i = 0; i < CmdBufs.size(); i++) {
			VkCommandBuffer& CmdBuf = CmdBufs[i];

			OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			OgldevVK::ImageMemBarrier(CmdBuf, m_vkCore.GetImage(i), m_vkCore.GetSwapChainFormat(),
				                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1, 0);

			BeginRendering(CmdBuf, i, FirstCommandBuffer);
		
			m_pipelines[LightingMode].Bind(i, CmdBuf, 
				m_modelContexts[MeshIndex].m_descSets[i], m_modelContexts[MeshIndex].m_baseTextureIndex);

			m_modelContexts[MeshIndex].m_pModel->RecordCommandBufferIndirect(CmdBuf);
			
			//m_skybox.RecordCommandBuffer(CmdBuf, i);

			vkCmdEndRendering(CmdBuf);

			if (WithSecondBarrier) {
				OgldevVK::ImageMemBarrier(CmdBuf, m_vkCore.GetImage(i), m_vkCore.GetSwapChainFormat(),
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, 1, 0);
			}

			VkResult res = vkEndCommandBuffer(CmdBuf);

			CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
		}

		printf("Command buffers recorded\n");
	}


	void BeginRendering(VkCommandBuffer CmdBuf, int ImageIndex, bool FirstCommandBuffer)
	{
        if (FirstCommandBuffer) {
			VkClearValue ClearColor = {
				.color = {1.0f, 0.0f, 0.0f, 1.0f},
			};

			VkClearValue DepthValue = {
				.depthStencil = {.depth = 1.0f, .stencil = 0 }
			};

			m_vkCore.BeginDynamicRendering(CmdBuf, ImageIndex, &ClearColor, &DepthValue);

		} else {
			m_vkCore.BeginDynamicRendering(CmdBuf, ImageIndex, NULL, NULL);
		}		
	}


	void UpdateGUI()
	{		
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_AlwaysAutoResize);   // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

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

		static const char* lightingModeNames[] = { "Unlit", "Normals", "Ambient", "Full" };
		for (int i = 0; i < OgldevVK::NUM_LIGHTING_MODES; ++i) {
			ImGui::RadioButton(lightingModeNames[i], (int*)&m_lightingMode, i);
		}

		ImGui::gizmo3D("##Dir1", m_lightDir, 200.0f, imguiGizmo::modeDirection);

		static int counter = 0;

		if (ImGui::Button("Button")) {                           // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		}

		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		ImGui::Render();
	}


	void UpdateUniformBuffers(int MeshIndex, int ImageIndex)
	{		
		glm::mat4 IndentityMatrix = glm::mat4(1.0f);

		glm::mat4 Scale = m_scale * glm::scale(IndentityMatrix, glm::vec3(Models[MeshIndex].Scale));

		glm::mat4 Translate = glm::translate(glm::mat4(1.0f), m_position + Models[MeshIndex].Pos);

		glm::mat4 World = Translate * Scale;

		glm::mat4 VP = m_pGameCamera->GetVPMatrix();

		glm::mat4 WVP = VP * World;

		glm::vec4 AmbientLight = glm::vec4(0.1, 0.12, 0.15, 1.0);
		glm::vec3 LightDirection = glm::vec3(-m_lightDir.x, -m_lightDir.y, -m_lightDir.z);
		//printf("Light dir: %f %f %f\n", LightDirection.x, LightDirection.y, LightDirection.z);
        // We don't care which pipeline is used to update the uniform buffers
		m_pipelines[0].UpdateUniformBuffers(ImageIndex, WVP, World, 
									m_modelContexts[MeshIndex].m_pModel->GetTransformations(), 
									AmbientLight,
	                                LightDirection, 
									m_modelContexts[MeshIndex].m_uniformBuffersVS, 
									m_modelContexts[MeshIndex].m_uniformBuffersFS);
		glm::mat4 VPNoTranslate = m_pGameCamera->GetVPMatrixNoTranslate();
		//m_skybox.Update(ImageIndex, VPNoTranslate);
	}

	GLFWwindow* m_pWindow = NULL;
	OgldevVK::VulkanCore m_vkCore;
	VkDescriptorPool m_descPool = VK_NULL_HANDLE;
	OgldevVK::VulkanQueue* m_pQueue = NULL;
	VkDevice m_device = NULL;
	int m_numImages = 0;
	struct CommandBuffersVecs {
		std::vector<VkCommandBuffer> WithGUI;
		std::vector<VkCommandBuffer> WithoutGUI;
	};
    std::vector<std::vector<CommandBuffersVecs>> m_cmdBufs;	// outer dim: meshes, inner dim: lighting modes
	VkShaderModule m_vs = VK_NULL_HANDLE;
	VkShaderModule m_fs = VK_NULL_HANDLE;
	OgldevVK::LightingProgram m_pipelines[OgldevVK::NUM_LIGHTING_MODES];
	//OgldevVK::Skybox m_skybox;
	std::vector<ModelContext> m_modelContexts;
	GLMCameraFirstPerson* m_pGameCamera = NULL;
	OgldevVK::ImGUIRenderer m_imGUIRenderer;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	BigTextureArray m_bigTextureArray;

	// GUI state
	bool m_showGui = false;
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);
	float m_scale = 0.1f;
	OgldevVK::LIGHTING_MODE m_lightingMode = OgldevVK::LIGHTING_MODE_FULL;
	vec3 m_lightDir = vec3(0.0f, 0.14f, 1.0f);
};


int main(int argc, char* argv[])
{
	VulkanApp App(WINDOW_WIDTH, WINDOW_HEIGHT);

	App.Init(APP_NAME);

	App.Execute();

	return 0;
}