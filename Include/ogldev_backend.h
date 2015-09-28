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

#ifndef OGLDEV_KEYS_H
#define OGLDEV_KEYS_H

#include "ogldev_types.h"
#include "ogldev_callbacks.h"

enum OGLDEV_BACKEND_TYPE {
    OGLDEV_BACKEND_TYPE_GLUT,
    OGLDEV_BACKEND_TYPE_GLFW
};


void OgldevBackendInit(OGLDEV_BACKEND_TYPE BackendType, int argc, char** argv, bool WithDepth, bool WithStencil);

void OgldevBackendTerminate();

bool OgldevBackendCreateWindow(uint Width, uint Height, bool isFullScreen, const char* pTitle);

void OgldevBackendRun(ICallbacks* pCallbacks);

void OgldevBackendLeaveMainLoop();

void OgldevBackendSwapBuffers();

void OgldevBackendSetMousePos(uint x, uint y);


#endif