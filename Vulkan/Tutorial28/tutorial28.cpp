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
		Tutorial #28: Compute Shaders
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
#include "Pipelines/ogldev_vulkan_texgen_pipeline.h"
#include "Pipelines/ogldev_vulkan_fsquad_pipeline.h"
#include "ogldev_vulkan_glfw.h"
#include "ogldev_glm_camera.h"
#include "ogldev_vulkan_imgui.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define APP_NAME "Tutorial 28"

static int CS_OUTPUT_WIDTH = 1280;
static int CS_OUTPUT_HEIGHT = 720;

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

struct UniformData {
	float time;
};

#if defined(_MSC_VER)
#pragma pack(pop)
#endif




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
		m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.WithGUI.size(), m_cmdBufs.WithGUI.data());
		m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.WithoutGUI.size(), m_cmdBufs.WithoutGUI.data());
		
		m_texGenComputePipeline.Destroy();

		m_fsQuadPipeline.Destroy();
		vkDestroyDescriptorPool(m_device, m_descPool, NULL);
			
		m_csOutput.Destroy(m_device);

		for (OgldevVK::BufferAndMemory& ubo : m_ubos) {
			ubo.Destroy(m_device);
		}

		m_imGUIRenderer.Destroy();
	}

	void Init(const char* pAppName)
	{
		m_pWindow = OgldevVK::glfw_vulkan_init(WINDOW_WIDTH, WINDOW_HEIGHT, pAppName);

		m_vkCore.Init(pAppName, m_pWindow, true, true);
		m_device = m_vkCore.GetDevice();
		m_numImages = m_vkCore.GetNumImages();
		m_pQueue = m_vkCore.GetQueue();

		VkImageUsageFlags Usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;

		m_vkCore.CreateTexture(m_csOutput, CS_OUTPUT_WIDTH, CS_OUTPUT_HEIGHT, Usage, Format);
		
		CreateDescriptorPool();
		CreatePipelines();
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

			VkCommandBuffer CmdBufs[] = { m_cmdBufs.WithGUI[ImageIndex], ImGUICmdBuf};

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
		glm::vec3 Target(-1.0f, -0.1f, 0.0f);
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


	void CreateDescriptorPool()
	{
		u32 TextureCount = 50;
		u32 UniformBufferCount = 50;
		u32 StorageBufferCount = 50;
		u32 MaxSets = m_numImages * 2;	// TexGen program and FS quad program

		m_descPool = m_vkCore.CreateDescPool(TextureCount, UniformBufferCount, StorageBufferCount, MaxSets);
	}

	
	void CreatePipelines()
	{
		CreateGraphicsPipeline();
		CreateComputePipeline();
	}


	void CreateGraphicsPipeline()
	{
		m_fsQuadPipeline.Init(m_vkCore, m_descPool, "../VulkanCore/Shaders/FSQuad.vert", "test.frag");
		m_fsQuadPipeline.AllocDescSets(m_numImages, m_fsQuadDescSets);
		m_fsQuadPipeline.UpdateDescSets(m_fsQuadDescSets, m_csOutput);
	}


	void CreateComputePipeline()
	{
		//m_texGenComputePipeline.Init(m_vkCore, m_descPool, "star_nest.comp");
		//m_texGenComputePipeline.Init(m_vkCore, m_descPool, "lets_self_reflect.comp");
		//m_texGenComputePipeline.Init(m_vkCore, m_descPool, "battered_alien_planet.comp");
		//m_texGenComputePipeline.Init(m_vkCore, m_descPool, "spiral_galaxy.comp");
		m_texGenComputePipeline.Init(m_vkCore, m_descPool, "flux_core.comp");
		m_ubos = m_vkCore.CreateUniformBuffers(sizeof(UniformData));
		m_texGenComputePipeline.AllocDescSets(m_numImages, m_texGenDescSets);
		m_texGenComputePipeline.UpdateDescSets(m_texGenDescSets, m_csOutput, m_ubos);
	}


	void RecordCommandBuffers()
	{
		RecordCommandBuffersInternal(true, m_cmdBufs.WithoutGUI);

		RecordCommandBuffersInternal(false, m_cmdBufs.WithGUI);
	}


	void RecordCommandBuffersInternal(bool WithSecondBarrier, std::vector<VkCommandBuffer>& CmdBufs)
	{
		for (uint i = 0; i < CmdBufs.size(); i++) {
			VkCommandBuffer& CmdBuf = CmdBufs[i];			

			OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			m_csOutput.ImageMemoryBarrier(CmdBuf,
										  VK_IMAGE_LAYOUT_GENERAL,                   // new layout
										  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,     // srcStage: last use was sampling
										  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);     // dstStage: next use is compute write

			m_texGenComputePipeline.RecordCommandBuffer(m_texGenDescSets[i], CmdBuf, CS_OUTPUT_WIDTH / 16, CS_OUTPUT_HEIGHT / 16, 1);

			m_csOutput.ImageMemoryBarrier(CmdBuf,
										  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,  // new layout
										  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,      // srcStage: last use was compute write
										  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);    // dstStage: next use is sampling

			OgldevVK::ImageMemBarrier(CmdBuf, m_vkCore.GetImage(i), m_vkCore.GetSwapChainFormat(),
				                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

			BeginRendering(CmdBuf, i);
		
			m_fsQuadPipeline.Bind(CmdBuf, m_fsQuadDescSets[i]);

			m_fsQuadPipeline.RecordCommandBuffer(CmdBuf);

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

		if (ImGui::Button("Button")) {  // Buttons return true when clicked (most widgets return true when edited/activated)
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
		UniformData ud = {
			.time = (float)glfwGetTime()
		};

		m_ubos[ImageIndex].Update(m_device, &ud, sizeof(ud));
	}

	GLFWwindow* m_pWindow = NULL;
	OgldevVK::VulkanCore m_vkCore;
	VkDescriptorPool m_descPool = VK_NULL_HANDLE;
	OgldevVK::VulkanQueue* m_pQueue = NULL;
	VkDevice m_device = NULL;
	int m_numImages = 0;
	struct {
		std::vector<VkCommandBuffer> WithGUI;
		std::vector<VkCommandBuffer> WithoutGUI;
	} m_cmdBufs;
	
	GLMCameraFirstPerson* m_pGameCamera = NULL;
	OgldevVK::ImGUIRenderer m_imGUIRenderer;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	bool m_showGui = false;
	glm::vec3 m_clearColor = glm::vec3(1.0f);
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);
	float m_scale = 0.1f;	

	OgldevVK::TexGenComputePipeline m_texGenComputePipeline;
	std::vector<VkDescriptorSet> m_texGenDescSets;
	std::vector<OgldevVK::BufferAndMemory> m_ubos;
	OgldevVK::VulkanTexture m_csOutput;
	
	OgldevVK::FullScreenQuadPipeline m_fsQuadPipeline;	
	std::vector<VkDescriptorSet> m_fsQuadDescSets;
};


int main(int argc, char* argv[])
{
	VulkanApp App(WINDOW_WIDTH, WINDOW_HEIGHT);

	App.Init(APP_NAME);

	App.Execute();

	return 0;
}