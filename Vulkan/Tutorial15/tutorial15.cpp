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
		Tutorial #15: Vertex Buffer Part 1
*/


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
#include "ogldev_vulkan_graphics_pipeline.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720


void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}


class VulkanApp
{
public:

	VulkanApp()
	{
	}

	~VulkanApp()
	{
		m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.size(), m_cmdBufs.data());
		m_vkCore.DestroyFramebuffers(m_frameBuffers);
		vkDestroyShaderModule(m_device, m_vs, NULL);
		vkDestroyShaderModule(m_device, m_fs, NULL);
		delete m_pPipeline;
		vkDestroyRenderPass(m_device, m_renderPass, NULL);
		vkDestroyBuffer(m_device, m_vb, NULL);
	}

	void Init(const char* pAppName, GLFWwindow* pWindow)
	{
		m_pWindow = pWindow;
		m_vkCore.Init(pAppName, pWindow);
		m_device = m_vkCore.GetDevice();
		m_numImages = m_vkCore.GetNumImages();
		m_pQueue = m_vkCore.GetQueue();
		m_renderPass = m_vkCore.CreateSimpleRenderPass();
		m_frameBuffers = m_vkCore.CreateFramebuffer(m_renderPass);
		CreateShaders();
		CreateVertexBuffer();
		CreatePipeline();
		CreateCommandBuffers();
		RecordCommandBuffers();
	}

	void RenderScene()
	{
		u32 ImageIndex = m_pQueue->AcquireNextImage();

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


	void CreateVertexBuffer()
	{
		struct Vertex {
			Vertex(const glm::vec3& p, const glm::vec2& t)
			{
				Pos = p;
				Tex = t;
			}

			glm::vec3 Pos;
			glm::vec2 Tex;
		};

		std::vector<Vertex> Vertices = {
			Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
			Vertex(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
			Vertex(glm::vec3(0.0f,  1.0f, 0.0f), glm::vec2(1.0f, 1.0f)) };

		m_vertexBufferSize = sizeof(Vertices[0]) * Vertices.size();
		m_vb = m_vkCore.CreateVertexBuffer(Vertices.data(), m_vertexBufferSize);
	}


	void CreateShaders()
	{
		m_vs = OgldevVK::CreateShaderModuleFromText(m_device, "test.vert");

		m_fs = OgldevVK::CreateShaderModuleFromText(m_device, "test.frag");
	}


	void CreatePipeline()
	{
		m_pPipeline = new OgldevVK::GraphicsPipeline(m_device, m_pWindow, m_renderPass, m_vs, m_fs, m_vb, m_vertexBufferSize, m_numImages);
	}


	void RecordCommandBuffers()
	{
		VkClearColorValue ClearColor = { 1.0f, 0.0f, 0.0f, 0.0f };
		VkClearValue ClearValue;
		ClearValue.color = ClearColor;

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
			.clearValueCount = 1,
			.pClearValues = &ClearValue
		};

		for (uint i = 0; i < m_cmdBufs.size(); i++) {
			OgldevVK::BeginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT); 

			RenderPassBeginInfo.framebuffer = m_frameBuffers[i];
	
			vkCmdBeginRenderPass(m_cmdBufs[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
			m_pPipeline->Bind(m_cmdBufs[i], i);

			u32 VertexCount = 3;
			u32 InstanceCount = 1;
			u32 FirstVertex = 0;
			u32 FirstInstance = 0;

			vkCmdDraw(m_cmdBufs[i], VertexCount, InstanceCount, FirstVertex, FirstInstance);

			vkCmdEndRenderPass(m_cmdBufs[i]);

			VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
			CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
		}

		printf("Command buffers recorded\n");
	}

	GLFWwindow* m_pWindow = NULL;
	OgldevVK::VulkanCore m_vkCore;
	OgldevVK::VulkanQueue* m_pQueue = NULL;
	VkDevice m_device = NULL;
	int m_numImages = 0;
	std::vector<VkCommandBuffer> m_cmdBufs;
	VkRenderPass m_renderPass;
	std::vector<VkFramebuffer> m_frameBuffers;
	VkShaderModule m_vs;
	VkShaderModule m_fs;
	OgldevVK::GraphicsPipeline* m_pPipeline = NULL;
	VkBuffer m_vb;
	size_t m_vertexBufferSize = 0;
};


#define APP_NAME "Tutorial 15"

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