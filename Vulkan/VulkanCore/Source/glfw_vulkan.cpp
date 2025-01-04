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

#include <vulkan/vulkan.h>

#include "ogldev_vulkan_glfw.h"

namespace OgldevVK {

static void GLFW_KeyCallback(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods)
{
	GLFWCallbacks* pGLFWCallbacks = (GLFWCallbacks*)glfwGetWindowUserPointer(pWindow);

	pGLFWCallbacks->Key(pWindow, Key, Scancode, Action, Mods);
}


static void GLFW_MouseCallback(GLFWwindow* pWindow, double xpos, double ypos)
{
	GLFWCallbacks* pGLFWCallbacks = (GLFWCallbacks*)glfwGetWindowUserPointer(pWindow);

	pGLFWCallbacks->MouseMove(pWindow, xpos, ypos);
}


static void GLFW_MouseButtonCallback(GLFWwindow* pWindow, int Button, int Action, int Mods)
{
	GLFWCallbacks* pGLFWCallbacks = (GLFWCallbacks*)glfwGetWindowUserPointer(pWindow);

	pGLFWCallbacks->MouseButton(pWindow, Button, Action, Mods);
}


GLFWwindow* glfw_vulkan_init(int Width, int Height, const char* pTitle)
{
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	if (!glfwVulkanSupported()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, 0);

	GLFWwindow* pWindow = glfwCreateWindow(Width, Height, pTitle, NULL, NULL);

	if (!pWindow) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	return pWindow;
}


void glfw_vulkan_set_callbacks(GLFWwindow* pWindow, GLFWCallbacks* pCallbacks)
{
	glfwSetWindowUserPointer(pWindow, pCallbacks);

	glfwSetKeyCallback(pWindow, GLFW_KeyCallback);
	glfwSetCursorPosCallback(pWindow, GLFW_MouseCallback);
	glfwSetMouseButtonCallback(pWindow, GLFW_MouseButtonCallback);
}

}