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
		Tutorial #20: Loading models using Assimp
*/

#include <array>
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_wrapper.h"
#include "ogldev_vulkan_shader.h"
#include "ogldev_vulkan_graphics_pipeline_v2.h"
#include "ogldev_vulkan_simple_mesh.h"
#include "ogldev_vulkan_glfw.h"
#include "ogldev_vulkan_model.h"
#include "ogldev_glm_camera.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080


class VulkanApp : public OgldevVK::GLFWCallbacks
{
public:

	VulkanApp(int WindowWidth, int WindowHeight) : m_model(false)
	{
		m_windowWidth = WindowWidth;
		m_windowHeight = WindowHeight;
	}

	~VulkanApp()
	{
		m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.size(), m_cmdBufs.data());
		m_vkCore.DestroyFramebuffers(m_frameBuffers);
		vkDestroyShaderModule(m_device, m_vs, NULL);
		vkDestroyShaderModule(m_device, m_fs, NULL);
		delete m_pPipeline;
		vkDestroyRenderPass(m_device, m_renderPass, NULL);
		m_model.Destroy();
	}

	void Init(const char* pAppName)
	{
		m_pWindow = OgldevVK::glfw_vulkan_init(WINDOW_WIDTH, WINDOW_HEIGHT, pAppName);

		m_vkCore.Init(pAppName, m_pWindow, true);
		m_device = m_vkCore.GetDevice();
		m_numImages = m_vkCore.GetNumImages();
		m_pQueue = m_vkCore.GetQueue();
		m_renderPass = m_vkCore.CreateSimpleRenderPass();
		m_frameBuffers = m_vkCore.CreateFramebuffers(m_renderPass);
		CreateShaders();
		CreateMesh();
		CreatePipeline();
		CreateCommandBuffers();
		RecordCommandBuffers();
		DefaultCreateCameraPers();
		// The object is ready to receive callbacks
		OgldevVK::glfw_vulkan_set_callbacks(m_pWindow, this);
	}


	void RenderScene()
	{
		u32 ImageIndex = m_pQueue->AcquireNextImage();

		UpdateUniformBuffers(ImageIndex);

		m_pQueue->SubmitAsync(m_cmdBufs[ImageIndex]);

		m_pQueue->Present(ImageIndex);
	}
	
	
	void Key(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods)
	{
		bool Handled = true;

		switch (Key) {
		case GLFW_KEY_ESCAPE:
		case GLFW_KEY_Q:
			glfwDestroyWindow(m_pWindow);
			glfwTerminate();
			exit(0);

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
		m_pGameCamera->HandleMouseButton(Button, Action, Mods);
	}
	
	
	void Execute()
	{
		float CurTime = (float)glfwGetTime();

		while (!glfwWindowShouldClose(m_pWindow)) {
			float Time = (float)glfwGetTime();
			float dt = Time - CurTime;
			m_pGameCamera->Update(dt);
			RenderScene();
			CurTime = Time;
			glfwPollEvents();
		}

		glfwTerminate();
	}


private:

	void DefaultCreateCameraPers()
	{
		float FOV = 45.0f;
		float zNear = 0.1f;
		float zFar = 2500.0f;

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

		glm::vec3 Pos(0.0f, 0.0f, -1.0f);
		glm::vec3 Target(0.0f, 0.0f, 1.0f);
		glm::vec3 Up(0.0, 1.0f, 0.0f);

		m_pGameCamera = new GLMCameraFirstPerson(Pos, Target, Up, persProjInfo);
	}


	void CreateCommandBuffers()
	{
		m_cmdBufs.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.data());

		printf("Created command buffers\n");
	}


	void CreateMesh()
	{
		m_model.Init(&m_vkCore);
	//	m_model.LoadAssimpModel("../../Content/bs_ears.obj");
		//m_model.LoadAssimpModel("../../Content/stanford_dragon_pbr/scene.gltf");
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
		VkFormat ColorFormat = m_vkCore.GetSwapChainFormat();		// forward compatibility with tutorial 21
		VkFormat DepthFormat = m_vkCore.GetDepthFormat();           // forward compatibility with tutorial 21
		m_pPipeline = new OgldevVK::GraphicsPipelineV2(m_device, m_pWindow, m_renderPass, m_vs, m_fs, m_numImages, ColorFormat, DepthFormat);
	}


	void RecordCommandBuffers()
	{
		std::array<VkClearValue, 2> ClearValues{};
		ClearValues[0].color = { {1.0f, 0.0f, 0.0f, 1.0f} };
		ClearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo RenderPassBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = NULL,
			.renderPass = m_renderPass,
			.renderArea = {
				.offset = {
					.x = 0,
					.y = 0
				},
				.extent = {
					.width = WINDOW_WIDTH,
					.height = WINDOW_HEIGHT
				}
			},
			.clearValueCount = (u32)ClearValues.size(),
			.pClearValues = ClearValues.data()
		};

		m_model.CreateDescriptorSets(*m_pPipeline);

		for (uint i = 0; i < m_cmdBufs.size(); i++) {
			VkCommandBuffer& CmdBuf = m_cmdBufs[i];

			OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

			RenderPassBeginInfo.framebuffer = m_frameBuffers[i];
	
			vkCmdBeginRenderPass(CmdBuf, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
			m_pPipeline->Bind(CmdBuf);

			m_model.RecordCommandBuffer(CmdBuf, *m_pPipeline, i);

			vkCmdEndRenderPass(CmdBuf);

			VkResult res = vkEndCommandBuffer(CmdBuf);
			CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
		}

		printf("Command buffers recorded\n");
	}


	void UpdateUniformBuffers(uint32_t ImageIndex)
	{		
		static float foo = 0.0f;

		glm::mat4 Rotate0 = glm::mat4(1.0);
		Rotate0 = glm::rotate(Rotate0, glm::radians(180.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)));

		glm::mat4 Rotate = glm::mat4(1.0);
		Rotate = glm::rotate(Rotate, glm::radians(foo), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));
		foo += 0.005f;

		glm::mat4 VP = m_pGameCamera->GetVPMatrix();

		glm::mat4 WVP = VP * Rotate * Rotate0;
		m_model.Update(ImageIndex, WVP);
	}

	GLFWwindow* m_pWindow = NULL;
	OgldevVK::VulkanCore m_vkCore;
	OgldevVK::VulkanQueue* m_pQueue = NULL;
	VkDevice m_device = NULL;
	int m_numImages = 0;
	std::vector<VkCommandBuffer> m_cmdBufs;
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> m_frameBuffers;
	VkShaderModule m_vs = VK_NULL_HANDLE;
	VkShaderModule m_fs = VK_NULL_HANDLE;
	OgldevVK::GraphicsPipelineV2* m_pPipeline = NULL;
	OgldevVK::VkModel m_model;
	GLMCameraFirstPerson* m_pGameCamera = NULL;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
};


#define APP_NAME "Tutorial 20"

int main(int argc, char* argv[])
{
	VulkanApp App(WINDOW_WIDTH, WINDOW_HEIGHT);

	App.Init(APP_NAME);

	App.Execute();

	return 0;
}