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

#pragma once

#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace OgldevVK {

class GLFWCallbacks {

public:

	virtual void Key(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods) = 0;

	virtual void MouseMove(GLFWwindow* pWindow, double xpos, double ypos) = 0;

	virtual void MouseButton(GLFWwindow* pWindow, int Button, int Action, int Mods) = 0;
};

// Step #1: initialize GLFW and create a window
GLFWwindow* glfw_vulkan_init(int Width, int Height, const char* pTitle);

// Step #2: initialize the GLFW callback mechanism
void glfw_vulkan_set_callbacks(GLFWwindow* pWindow, GLFWCallbacks* pCallbacks);

}
