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
		Tutorial #20: Immediate Mode Canvas
*/


#include <stdio.h>
#include <stdlib.h>
#include <array>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ogldev_math_3d.h"
#include "ogldev_glm_camera.h"
#include "ogldev_vulkan_glfw.h"
#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_wrapper.h"
#include "ogldev_vulkan_shader.h"
#include "ogldev_vulkan_graphics_pipeline.h"
#include "ogldev_vulkan_clear.h"
#include "ogldev_vulkan_finish.h"
#include "ogldev_vulkan_model.h"
#include "ogldev_vulkan_canvas.h"
#include "ogldev_vulkan_imgui.h"

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000


struct UniformData {
	Matrix4f WVP;
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
		m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.size(), m_cmdBufs.data());

		if (m_pImGUIRenderer) {
			delete m_pImGUIRenderer;
		}

		if (m_pFinishRenderer) {
			delete m_pFinishRenderer;
		}

		if (m_p2DCanvas) {
			delete m_p2DCanvas;
		}

		if (m_pModelRenderer) {
			delete m_pModelRenderer;
		}
		
		if (m_pClearRenderer) {
			delete m_pClearRenderer;
		}
	}

	void Init(const char* pAppName)
	{
		m_pWindow = OgldevVK::glfw_vulkan_init(WINDOW_WIDTH, WINDOW_HEIGHT, pAppName);

		m_vkCore.Init(pAppName, m_pWindow);
		m_device = m_vkCore.GetDevice();
		m_numImages = m_vkCore.GetNumImages();
		m_pQueue = m_vkCore.GetQueue();
		
		m_pModelRenderer = new OgldevVK::ModelRenderer(m_vkCore, "../../Content/rubber_duck/scene.gltf", "../../Content/rubber_duck/textures/Duck_baseColor.png", sizeof(UniformData));
		m_pClearRenderer = new OgldevVK::ClearRenderer(m_vkCore);
		//m_p2DCanvas = new OgldevVK::CanvasRenderer(m_vkCore);

#if 1	// Enable either the ImGUI renderer OR the finish renderer
		m_pImGUIRenderer = new OgldevVK::ImGUIRenderer(m_vkCore);
#else
		m_pFinishRenderer = new OgldevVK::FinishRenderer(m_vkCore);
#endif				
		if (m_p2DCanvas) {    // The check is required when the canvas creation is commented out
			m_p2DCanvas->Plane3D(glm::vec3(0, 0.25f, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), 40, 40, 10.0f, 10.0f, glm::vec4(1, 0, 0, 1), glm::vec4(1, 0, 0.1f, 1));

			//m_p2DCanvas->Line(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f, 0.0, 0.0, 1.0f));

			for (int i = 0; i < m_numImages; i++) {
				m_p2DCanvas->UpdateBuffer(i);
			}
		}

		CreateCommandBuffers();
		RecordCommandBuffers();	

		// The object is ready to receive callbacks
		OgldevVK::glfw_vulkan_set_callbacks(m_pWindow, this);
	}


	void RenderScene()
	{
		u32 ImageIndex = m_pQueue->AcquireNextImage();

		UpdateUniformBuffers(ImageIndex);

		m_pQueue->SubmitAsync(m_cmdBufs[ImageIndex]);

		if (m_pImGUIRenderer) {
			m_pImGUIRenderer->OnFrame(ImageIndex);
		}

		m_pQueue->Present(ImageIndex);
	}


	void Key(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods)
	{
		bool Press = Action != GLFW_RELEASE;

		switch (Key) {
		case GLFW_KEY_ESCAPE:
			if (Press) {
				glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
			}
			break;

		case GLFW_KEY_W:
			m_cameraPositioner.m_movement.Forward = Press;
			//printf("1 %d\n", m_cameraPositioner.m_movement.Forward);
			break;

		case GLFW_KEY_S:
			m_cameraPositioner.m_movement.Backward = Press;
			break;

		case GLFW_KEY_A:
			m_cameraPositioner.m_movement.Left = Press;
			break;

		case GLFW_KEY_D:
			m_cameraPositioner.m_movement.Right = Press;
			break;

		case GLFW_KEY_PAGE_UP:
			m_cameraPositioner.m_movement.Up = Press;
			break;

		case GLFW_KEY_PAGE_DOWN:
			m_cameraPositioner.m_movement.Down = Press;
			break;

		case GLFW_KEY_SPACE:
			m_cameraPositioner.SetUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
			break;
		}

		if (Mods & GLFW_MOD_SHIFT) {
			m_cameraPositioner.m_movement.FastSpeed = Press;
		}
	}


	void MouseMove(GLFWwindow* pWindow, double xpos, double ypos)
	{
		int Width, Height;

		glfwGetWindowSize(pWindow, &Width, &Height);

		m_mouseState.m_pos.x = (float)xpos / (float)Width;
		m_mouseState.m_pos.y = (float)ypos / (float)Height;
	}


	void MouseButton(GLFWwindow* pWindow, int Button, int Action, int Mods)
	{
		if (Button == GLFW_MOUSE_BUTTON_LEFT) {
			m_mouseState.m_pressedLeft = (Action == GLFW_PRESS);
		}
	}


	void Execute()
	{
		glfwSetWindowUserPointer(m_pWindow, this);

		float CurTime = (float)glfwGetTime();

		while (!glfwWindowShouldClose(m_pWindow)) {
			float Time = (float)glfwGetTime();
			float dt = Time - CurTime;
			m_cameraPositioner.Update(dt, m_mouseState.m_pos, m_mouseState.m_pressedLeft);
			RenderScene();
			CurTime = Time;
			glfwPollEvents();
		}

		glfwTerminate();
	}


private:

	void CreateCommandBuffers()
	{
		m_cmdBufs.resize(m_numImages);
		m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.data());

		printf("Created command buffers\n");
	}


	void RecordCommandBuffers()
	{
		for (int i = 0; i < m_numImages; i++) {
			RecordCommandBuffer(i);
		}
	}


	void RecordCommandBuffer(int ImageIndex)
	{
		VkCommandBuffer CmdBuf = m_cmdBufs[ImageIndex];

		OgldevVK::BeginCommandBuffer(CmdBuf, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		if (m_pClearRenderer) m_pClearRenderer->FillCommandBuffer(CmdBuf, ImageIndex);
		if (m_pModelRenderer) m_pModelRenderer->FillCommandBuffer(CmdBuf, ImageIndex);
		if (m_p2DCanvas) m_p2DCanvas->FillCommandBuffer(CmdBuf, ImageIndex);
		if (m_pImGUIRenderer) m_pImGUIRenderer->FillCommandBuffer(CmdBuf, ImageIndex);
		if (m_pFinishRenderer) m_pFinishRenderer->FillCommandBuffer(CmdBuf, ImageIndex);

		CHECK_VK_RESULT(vkEndCommandBuffer(CmdBuf), "vkEndCommandBuffer");
	}


	void UpdateUniformBuffers(uint32_t ImageIndex)
	{
		glm::mat4 Translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.5f, -2.0f));
		glm::mat4 Rotation = glm::rotate(glm::mat4(1.f), glm::pi<float>(), glm::vec3(1, 0, 0));
		glm::mat4 World = glm::rotate(Translation * Rotation, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 View = m_cameraPositioner.GetViewMatrix();

		float ar = (float)m_windowWidth / (float)m_windowHeight;

		glm::mat4 Proj = glm::perspective(45.0f, ar, 0.1f, 1000.0f);
		glm::mat4 WVP = Proj * View * World;

		if (m_pModelRenderer) m_pModelRenderer->UpdateUniformBuffers(ImageIndex, &WVP[0][0], sizeof(WVP));
		if (m_p2DCanvas) m_p2DCanvas->UpdateUniformBuffer(ImageIndex, WVP[0][0], (float)glfwGetTime());
	}

	GLFWwindow* m_pWindow = NULL;
	OgldevVK::VulkanCore m_vkCore;
	OgldevVK::VulkanQueue* m_pQueue = NULL;
	VkDevice m_device = NULL;
	int m_numImages = 0;
	std::vector<VkCommandBuffer> m_cmdBufs;
	OgldevVK::ClearRenderer* m_pClearRenderer = NULL;
	OgldevVK::FinishRenderer* m_pFinishRenderer = NULL;
	OgldevVK::ModelRenderer* m_pModelRenderer = NULL;
	OgldevVK::CanvasRenderer* m_p2DCanvas = NULL;
	OgldevVK::ImGUIRenderer* m_pImGUIRenderer = NULL;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	MouseState m_mouseState;
	CameraPositionerFirstPerson m_cameraPositioner = CameraPositionerFirstPerson(glm::vec3(0.0f),
																				 glm::vec3(0.0f, 0.0f, -1.0f),
																				 glm::vec3(0.0f, 1.0f, 0.0f));
};


#define APP_NAME "Tutorial 20"

int main(int argc, char* argv[])
{
	VulkanApp App(WINDOW_WIDTH, WINDOW_HEIGHT);

	App.Init(APP_NAME);

	App.Execute();

	return 0;
}