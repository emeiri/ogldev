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
		Tutorial #18: Loading meshes with Assimp
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

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000


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
		m_vkCore.DestroyTexture(m_texture);
		vkDestroyRenderPass(m_device, m_renderPass, NULL);
		m_mesh.Destroy();

		for (int i = 0; i < m_uniformBuffers.size(); i++) {
			m_uniformBuffers[i].Destroy(m_device);
		}

		delete m_pPipeline;
	}

	void Init(const char* pAppName, GLFWwindow* pWindow)
	{
		m_pWindow = pWindow;
		m_vkCore.Init(pAppName, pWindow);
		m_device = m_vkCore.GetDevice();
		m_numImages = m_vkCore.GetNumImages();
		m_pQueue = m_vkCore.GetQueue();
		m_renderPass = m_vkCore.CreateSimpleRenderPass(true);
		m_frameBuffers = m_vkCore.CreateFramebuffer(m_renderPass);

		CreateShaders();
		LoadMesh();
		CreateUniformBuffers();
		CreateTexture();		
		CreatePipeline();
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


	void LoadMesh()
	{
		m_mesh = m_vkCore.LoadSimpleMesh("../../Content/box.obj");
	}


	struct UniformData {
		Matrix4f WVP;
	};

	void CreateUniformBuffers()
	{
		m_uniformBuffers = m_vkCore.CreateUniformBuffers(sizeof(UniformData));
	}


	void CreateShaders()
	{
		m_vs = OgldevVK::CreateShaderModuleFromText(m_device, "test.vert");

		m_fs = OgldevVK::CreateShaderModuleFromText(m_device, "test.frag");
	}

	
	void CreateTexture()
	{
		m_vkCore.CreateTexture("../../Content/bricks.jpg", m_texture);
	}


	void CreatePipeline()
	{
		m_pPipeline = new OgldevVK::GraphicsPipeline(m_device, m_pWindow, m_renderPass, m_vs, m_fs, 
													 m_mesh.m_vb.m_buffer, m_mesh.m_vertexBufferSize, 
													 m_mesh.m_ib.m_buffer, m_mesh.m_indexBufferSize, 
													 m_numImages, m_uniformBuffers, sizeof(UniformData), &m_texture);
	}


	void RecordCommandBuffers()
	{
		VkClearColorValue ClearColor = { 1.0f, 0.0f, 0.0f, 0.0f };
		VkClearDepthStencilValue DepthStenciValue = {
			.depth = 1.0f,
			.stencil = 0
		};

		const std::array<VkClearValue, 2> ClearValues =	{
			VkClearValue {.
				color = ClearColor 
			},
			VkClearValue {
				.depthStencil = DepthStenciValue 
			}
		};

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

		for (uint i = 0; i < m_cmdBufs.size(); i++) {
			OgldevVK::BeginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT); 

			RenderPassBeginInfo.framebuffer = m_frameBuffers[i];
	
			vkCmdBeginRenderPass(m_cmdBufs[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
			m_pPipeline->Bind(m_cmdBufs[i], i);

			vkCmdDraw(m_cmdBufs[i], m_mesh.m_numIndices, 1, 0, 0);

			vkCmdEndRenderPass(m_cmdBufs[i]);

			VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
			CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
		}

		printf("Command buffers recorded\n");
	}


	void UpdateUniformBuffers(uint32_t ImageIndex)
	{
		glm::mat4 Translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.5f, -5.0f));
		glm::mat4 Rotation = glm::rotate(glm::mat4(1.f), glm::pi<float>(), glm::vec3(1, 0, 0));
		glm::mat4 World = glm::rotate(Translation * Rotation, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		
		int Width, Height;
		glfwGetFramebufferSize(m_pWindow, &Width, &Height);
		float ar = (float)Width / (float)Height;

		glm::mat4 Proj = glm::perspective(45.0f, ar, 0.1f, 1000.0f);
		glm::mat4 WVP = Proj * World;

		m_uniformBuffers[ImageIndex].Update(m_device, &WVP, sizeof(glm::mat4));
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
	OgldevVK::SimpleMesh m_mesh;
	std::vector<OgldevVK::BufferAndMemory> m_uniformBuffers;
	OgldevVK::VulkanTexture m_texture;	
};


#define APP_NAME "Tutorial 18"

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