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

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ogldev_math_3d.h"
#include "ogldev_vulkan_util.h"
#include "ogldev_vulkan_core.h"
#include "ogldev_vulkan_wrapper.h"
#include "ogldev_vulkan_shader.h"
#include "ogldev_vulkan_graphics_pipeline.h"
#include "ogldev_vulkan_clear.h"
#include "ogldev_vulkan_finish.h"
#include "ogldev_vulkan_model.h"
#include "ogldev_vulkan_canvas.h"

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000


void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

struct UniformData {
	Matrix4f WVP;
};



class VulkanApp
{
public:

	VulkanApp()
	{
	}

	~VulkanApp()
	{
		m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.size(), m_cmdBufs.data());
	}

	void Init(const char* pAppName, GLFWwindow* pWindow)
	{
		m_pWindow = pWindow;
		glfwGetFramebufferSize(m_pWindow, &m_windowWidth, &m_windowHeight);

		m_vkCore.Init(pAppName, pWindow);
		m_device = m_vkCore.GetDevice();
		m_numImages = m_vkCore.GetNumImages();
		m_pQueue = m_vkCore.GetQueue();
		
		m_pModelRenderer = new OgldevVK::ModelRenderer(m_vkCore, "../../Content/rubber_duck/scene.gltf", 
													   "../../Content/rubber_duck/textures/Duck_baseColor.png", sizeof(UniformData));
		m_pClearRenderer = new OgldevVK::ClearRenderer(m_vkCore);
		m_pFinishRenderer = new OgldevVK::FinishRenderer(m_vkCore);
		m_p2DCanvas = new OgldevVK::CanvasRenderer(m_vkCore);

		//m_p2DCanvas->Plane3D(glm::vec3(0, +1.5, 0), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), 40, 40, 10.0f, 10.0f, glm::vec4(1, 0, 0, 1), glm::vec4(1, 0, 0.1f, 1));

		m_p2DCanvas->Line(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f, 0.0, 0.0, 1.0f));

		for (int i = 0; i < m_numImages; i++) {
			m_p2DCanvas->UpdateBuffer(i);
		}

		CreateCommandBuffers();
		RecordCommandBuffers();		
	}

	void RenderScene()
	{
		u32 ImageIndex = m_pQueue->AcquireNextImage();

		UpdateUniformBuffers(ImageIndex);

		m_pQueue->SubmitAsync(m_cmdBufs[ImageIndex]);

		m_pQueue->Present(ImageIndex);
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

		m_pClearRenderer->FillCommandBuffer(CmdBuf, ImageIndex);
		m_pModelRenderer->FillCommandBuffer(CmdBuf, ImageIndex);
		m_p2DCanvas->FillCommandBuffer(CmdBuf, ImageIndex);
		m_pFinishRenderer->FillCommandBuffer(CmdBuf, ImageIndex);

		CHECK_VK_RESULT(vkEndCommandBuffer(CmdBuf), "vkEndCommandBuffer");
	}


	void UpdateUniformBuffers(uint32_t ImageIndex)
	{
		glm::mat4 Translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.5f, -2.0f));
		glm::mat4 Rotation = glm::rotate(glm::mat4(1.f), glm::pi<float>(), glm::vec3(1, 0, 0));
		glm::mat4 World = glm::rotate(Translation * Rotation, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		
		float ar = (float)m_windowWidth / (float)m_windowHeight;

		glm::mat4 Proj = glm::perspective(45.0f, ar, 0.1f, 1000.0f);
		glm::mat4 WVP = Proj * World;

		m_pModelRenderer->UpdateUniformBuffers(ImageIndex, &WVP[0][0], sizeof(WVP));
		m_p2DCanvas->UpdateUniformBuffer(ImageIndex, WVP[0][0], (float)glfwGetTime());
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
	int m_windowWidth = 0;
	int m_windowHeight = 0;
};


#define APP_NAME "Tutorial 20"

int main(int argc, char* argv[])
{
	if (!glfwInit()) {
		return 1;
	}

	if (!glfwVulkanSupported()) {
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, NULL, NULL);
	
	if (!pWindow) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(pWindow, GLFW_KeyCallback);

	VulkanApp App;
	App.Init(APP_NAME, pWindow);

	while (!glfwWindowShouldClose(pWindow)) {
		App.RenderScene();
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}