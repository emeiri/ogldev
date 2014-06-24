/*

	Copyright 2014 Etay Meiri

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

#ifdef WIN32
#include <Windows.h>
#endif
#include <stdio.h>
#include <GLFW/glfw3.h>

#include "ogldev_glfw_backend.h"

void GLFWErrorCallback(int error, const char* description)
{
#ifdef WIN32
	char msg[1000];
	_snprintf_s(msg, sizeof(msg), "GLFW error %d - %s", error, description);
	MessageBoxA(NULL, msg, NULL, 0);
#else
	fprintf(stderr, "GLFW error %d - %s", error, description);
#endif    

}

bool GLFWBackendInit()
{
	bool ret = false;

	if (glfwInit() == 1) {
		glfwSetErrorCallback(GLFWErrorCallback);
		ret = true;
	}

	return ret;
}


void GLFWBackendTerminate()
{
	glfwTerminate();
}


bool GLFWBackendCreateWindow(unsigned int Width, unsigned int Height, unsigned int bpp, bool isFullScreen, const char* pTitle)
{
	GLFWmonitor* pMonitor = isFullScreen ? glfwGetPrimaryMonitor() : NULL;

	GLFWwindow* pWindow = glfwCreateWindow(Width, Height, pTitle, pMonitor, NULL);

	if (pWindow) {
		glfwMakeContextCurrent(pWindow);

		while (!glfwWindowShouldClose(pWindow))
		{
			glfwSwapBuffers(pWindow);
			glfwPollEvents();
		}
	}

	return pWindow != NULL;
}