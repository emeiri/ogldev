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
*/


#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_core.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

GLFWwindow* window = NULL;

void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}


class VulkanApp
{
public:

	VulkanApp(GLFWwindow* pWindow) : m_vkCore(pWindow)
	{
	}

	~VulkanApp()
	{
	}

	void Init(const char* pAppName)
	{
		m_vkCore.Init(pAppName);
		CreateCommandBuffer();
		RecordCommandBuffers();
	}

	void RenderScene()
	{

	}

private:

	void CreateCommandBuffer()
	{
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.queueFamilyIndex = m_vkCore.GetQueue();

		VkResult res = vkCreateCommandPool(m_vkCore.GetDevice(), &cmdPoolCreateInfo, NULL, &m_cmdBufPool);
		CHECK_VK_RESULT(res, "vkCreateCommandPool\n");

		printf("Command buffer pool created\n");

		VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
		cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocInfo.commandPool = m_cmdBufPool;
		cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAllocInfo.commandBufferCount = m_vkCore.GetNumImages();

		m_cmdBufs.resize(m_vkCore.GetNumImages());

		res = vkAllocateCommandBuffers(m_vkCore.GetDevice(), &cmdBufAllocInfo, &m_cmdBufs[0]);
		CHECK_VK_RESULT(res, "vkAllocateCommandBuffers\n");

		printf("Created command buffers\n");
	}

	void RecordCommandBuffers()
	{

	}

	OgldevVK::VulkanCore m_vkCore;
	VkCommandPool m_cmdBufPool;
	std::vector<VkCommandBuffer> m_cmdBufs;
};


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

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 01", NULL, NULL);
	
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, GLFW_KeyCallback);

	VulkanApp App(window);
	App.Init("Tutorial 01");

	while (!glfwWindowShouldClose(window)) {
		App.RenderScene();
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}