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

	Vulkan For Beginners - 
		Tutorial #26: Descriptor Indexing
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
#include "ogldev_vulkan_graphics_pipeline_v3.h"
#include "ogldev_vulkan_glfw.h"
#include "ogldev_vulkan_model.h"
#include "ogldev_vulkan_skybox.h"
#include "ogldev_glm_camera.h"
#include "ogldev_vulkan_imgui.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define APP_NAME "Tutorial 26"

class VulkanApp : public OgldevVK::GLFWCallbacks
{
public:

	VulkanApp(int WindowWidth, int WindowHeight) : m_model(true)
	{
		m_windowWidth = WindowWidth;
		m_windowHeight = WindowHeight;
	}

	~VulkanApp()
	{
		m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.WithGUI.size(), m_cmdBufs.WithGUI.data());
		m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.WithoutGUI.size(), m_cmdBufs.WithoutGUI.data());
		vkDestroyShaderModule(m_device, m_vs, NULL);
		vkDestroyShaderModule(m_device, m_fs, NULL);

		delete m_pPipeline;
			
		m_model.Destroy();

		m_skybox.Destroy();

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
		CreateMesh();
		m_skybox.Init(&m_vkCore, "../../Content/textures/evening_road_01_puresky_8k_2.jpg");
		CreatePipeline();
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

		UpdateUniformBuffers(ImageIndex);

		if (m_showGui) {		
			UpdateGUI();

			VkCommandBuffer ImGUICmdBuf = m_imGUIRenderer.PrepareCommandBuffer(ImageIndex);

			VkCommandBuffer CmdBufs[] = { m_cmdBufs.WithGUI[ImageIndex], ImGUICmdBuf };

			m_pQueue->SubmitAsync(&CmdBufs[0], 2);
		} else {
			m_pQueue->SubmitAsync(m_cmdBufs.WithoutGUI[ImageIndex]);
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
		
		glm::vec3 Pos(0.0f, 0.0f, -0.33f);
		glm::vec3 Target(0.0f, 0.0f, 1.0f);
		glm::vec3 Up(0.0, 1.0f, 0.0f);

		m_pGameCamera = new GLMCameraFirstPerson(Pos, Target, Up, persProjInfo);
	}


	void CreateCommandBuffers()
	{
		m_cmdBufs.WithGUI.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.WithGUI.data());

		m_cmdBufs.WithoutGUI.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.WithoutGUI.data());

		printf("Created command buffers\n");
	}


	void CreateMesh()
	{
		m_model.Init(&m_vkCore);
	//	m_model.LoadAssimpModel("../../Content/bs_ears.obj");
	//	m_model.LoadAssimpModel("../../Content/stanford_dragon_pbr/scene.gltf");
	//	m_model.LoadAssimpModel("../../Content/stanford_armadillo_pbr/scene.gltf");
		m_model.LoadAssimpModel("../../Content/crytek_sponza/sponza.obj");
	//	m_model.LoadAssimpModel("../../Content/demolition/box_and_sphere.obj");
	//	m_model.LoadAssimpModel("../../Content/DamagedHelmet/DamagedHelmet.gltf");
	//	m_model.LoadAssimpModel("G:/emeir/Books/3D-Graphics-Rendering-Cookbook-2/deps/src/glTF-Sample-Models/2.0/WaterBottle/glTF/WaterBottle.gltf");
	}


	struct UniformData {
		glm::mat4 WVP;
	};


	void CreateShaders()
	{
		m_vs = OgldevVK::CreateShaderModuleFromText(m_device, "test.vert");

		m_fs = OgldevVK::CreateShaderModuleFromText(m_device, "test.frag");
	}


	void CreatePipeline()
	{
		OgldevVK::PipelineDesc pd;
		pd.Device = m_device;
		pd.pWindow = m_pWindow;
		pd.vs = m_vs;
		pd.fs = m_fs;
		pd.NumImages = m_numImages;
		pd.ColorFormat = m_vkCore.GetSwapChainFormat();
		pd.DepthFormat = m_vkCore.GetDepthFormat();
		pd.IsIB = true;
		pd.IsVB = true;
		pd.IsTex2D = true;
		pd.IsUniform = true;

		m_pPipeline = new OgldevVK::GraphicsPipelineV3(pd);
	}


	void RecordCommandBuffers()
	{
		m_model.CreateDescriptorSets(*m_pPipeline);

		RecordCommandBuffersInternal(true, m_cmdBufs.WithoutGUI);

		RecordCommandBuffersInternal(false, m_cmdBufs.WithGUI);
	}


	void RecordCommandBuffersInternal(bool WithSecondBarrier, std::vector<VkCommandBuffer>& CmdBufs)
	{
		for (uint i = 0; i < CmdBufs.size(); i++) {
			VkCommandBuffer& CmdBuf = CmdBufs[i];

			OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			OgldevVK::ImageMemBarrier(CmdBuf, m_vkCore.GetImage(i), m_vkCore.GetSwapChainFormat(),
				                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

			BeginRendering(CmdBuf, i);
		
			m_pPipeline->Bind(CmdBuf);
			m_model.RecordCommandBuffer(CmdBuf, *m_pPipeline, i);
			
			m_skybox.RecordCommandBuffer(CmdBuf, i);

			vkCmdEndRendering(CmdBuf);

			if (WithSecondBarrier) {
				OgldevVK::ImageMemBarrier(CmdBuf, m_vkCore.GetImage(i), m_vkCore.GetSwapChainFormat(),
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1);
			}

			VkResult res = vkEndCommandBuffer(CmdBuf);

			CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
		}

		printf("Command buffers recorded\n");
	}


	void BeginRendering(VkCommandBuffer CmdBuf, int ImageIndex)
	{
		VkClearValue ClearColor = {
			.color = {1.0f, 0.0f, 0.0f, 1.0f},
		};

		VkClearValue DepthValue = {
			.depthStencil = {.depth = 1.0f, .stencil = 0 }
		};

		m_vkCore.BeginDynamicRendering(CmdBuf, ImageIndex, &ClearColor, &DepthValue);
	}


	void UpdateGUI()
	{		
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_AlwaysAutoResize);   // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

		static float f = 0.0f;
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("Clear color", (float*)&m_clearColor); // Edit 3 floats representing a color

		ImGui::Begin("Transform");

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

		// For imGuIZMO, declare static or global variable or member class quaternion
		static quat qRot1 = quat(1.f, 0.f, 0.f, 0.f);
		static quat qRot2 = quat(1.f, 0.f, 0.f, 0.f);
		ImGui::gizmo3D("##gizmo1", qRot1, 200.0f/*, mode */);

		static vec4 dir(1.0f, 0.0f, 0.0f, 0.0f);
		ImGui::gizmo3D("##Dir1", dir, 200.0f, imguiGizmo::modeDirection);

		static int counter = 0;

		if (ImGui::Button("Button")) {                           // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		}

		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
		ImGui::End();

		ImGui::Render();
	}


	void UpdateUniformBuffers(uint32_t ImageIndex)
	{		
		glm::mat4 Scale = glm::scale(glm::mat4(0.01f), glm::vec3(m_scale));

		glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
		glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
		glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
		glm::mat4 Rotate = rotX * rotY * rotZ;
		glm::mat4 Rotate2 = glm::rotate(Rotate, glm::radians(180.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)));	// hack

		glm::mat4 Translate = glm::translate(glm::mat4(1.0f), m_position);

		glm::mat4 VP = m_pGameCamera->GetVPMatrix();

		glm::mat4 WVP = VP * Translate * Rotate2 * Scale;

		m_model.Update(ImageIndex, WVP);

		/*static float foo = 0.0f;
		foo += 0.000075f;
		glm::vec3 r(sinf(foo), sinf(foo) * 0.4f, cosf(foo));
		m_pGameCamera->SetTarget(r);*/

		glm::mat4 VPNoTranslate = m_pGameCamera->GetVPMatrixNoTranslate();
		m_skybox.Update(ImageIndex, VPNoTranslate);
	}

	GLFWwindow* m_pWindow = NULL;
	OgldevVK::VulkanCore m_vkCore;
	OgldevVK::VulkanQueue* m_pQueue = NULL;
	VkDevice m_device = NULL;
	int m_numImages = 0;
	struct {
		std::vector<VkCommandBuffer> WithGUI;
		std::vector<VkCommandBuffer> WithoutGUI;
	} m_cmdBufs;
	
	VkShaderModule m_vs = VK_NULL_HANDLE;
	VkShaderModule m_fs = VK_NULL_HANDLE;
	OgldevVK::GraphicsPipelineV3* m_pPipeline = NULL;	
	OgldevVK::Skybox m_skybox;
	OgldevVK::VkModel m_model;
	GLMCameraFirstPerson* m_pGameCamera = NULL;
	OgldevVK::ImGUIRenderer m_imGUIRenderer;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	bool m_showGui = false;
	glm::vec3 m_clearColor = glm::vec3(1.0f);
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);
	float m_scale = 0.1f;
};


int main(int argc, char* argv[])
{
	VulkanApp App(WINDOW_WIDTH, WINDOW_HEIGHT);

	App.Init(APP_NAME);

	App.Execute();

	return 0;
}