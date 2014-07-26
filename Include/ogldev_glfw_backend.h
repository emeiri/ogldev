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

#ifndef OGLDEV_GLFW_BACKEND_H
#define	OGLDEV_GLFW_BACKEND_H

#include "ogldev_types.h"
#include "ogldev_callbacks.h"

void GLFWBackendInit(int argc, char** argv, bool WithDepth, bool WithStencil);

void GLFWBackendTerminate();

bool GLFWBackendCreateWindow(uint Width, uint Height, bool isFullScreen, const char* pTitle);

void GLFWBackendRun(ICallbacks* pCallbacks);

void GLFWBackendSwapBuffers();

void GLFWBackendLeaveMainLoop();

#endif
