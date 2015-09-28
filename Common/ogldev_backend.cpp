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

#include <AntTweakBar.h>

#include "ogldev_util.h"
#include "ogldev_backend.h"
#include "ogldev_glut_backend.h"
#include "ogldev_glfw_backend.h"


static OGLDEV_BACKEND_TYPE sBackendType = OGLDEV_BACKEND_TYPE_GLUT;

void OgldevBackendInit(OGLDEV_BACKEND_TYPE BackendType, int argc, char** argv, bool WithDepth, bool WithStencil)
{
    sBackendType = BackendType;
    
    switch (BackendType) {
        case OGLDEV_BACKEND_TYPE_GLUT:
            GLUTBackendInit(argc, argv, WithDepth, WithStencil);
            break;
        case OGLDEV_BACKEND_TYPE_GLFW:
            GLFWBackendInit(argc, argv, WithDepth, WithStencil);
            break;
        default:
            assert(0);
    }   
}


void OgldevBackendTerminate()
{
    switch (sBackendType) {
        case OGLDEV_BACKEND_TYPE_GLUT:
            // nothing to do here...
            break;
        case OGLDEV_BACKEND_TYPE_GLFW:
            GLFWBackendTerminate();
            break;
        default:
            assert(0);
    }
}


bool OgldevBackendCreateWindow(uint Width, uint Height, bool isFullScreen, const char* pTitle)
{
    TwWindowSize(Width, Height);
    
    switch (sBackendType) {
        case OGLDEV_BACKEND_TYPE_GLUT:
            return GLUTBackendCreateWindow(Width, Height, isFullScreen, pTitle);
        case OGLDEV_BACKEND_TYPE_GLFW:
            return GLFWBackendCreateWindow(Width, Height, isFullScreen, pTitle);
        default:
            assert(0);
    }
    
    

	return false;
}


void OgldevBackendRun(ICallbacks* pCallbacks)
{  
    switch (sBackendType) {
        case OGLDEV_BACKEND_TYPE_GLUT:
            GLUTBackendRun(pCallbacks);
            break;
        case OGLDEV_BACKEND_TYPE_GLFW:
            GLFWBackendRun(pCallbacks);
            break;
        default:
            assert(0);
    }    
}


void OgldevBackendSwapBuffers()
{
    TwDraw();
    
    switch (sBackendType) {
        case OGLDEV_BACKEND_TYPE_GLUT:
            GLUTBackendSwapBuffers();
            break;
        case OGLDEV_BACKEND_TYPE_GLFW:
            GLFWBackendSwapBuffers();
            break;
        default:
            assert(0);
    }    
}


void OgldevBackendLeaveMainLoop()
{
    switch (sBackendType) {
        case OGLDEV_BACKEND_TYPE_GLUT:
            GLUTBackendLeaveMainLoop();
            break;
        case OGLDEV_BACKEND_TYPE_GLFW:
            GLFWBackendLeaveMainLoop();
            break;
        default:
            assert(0);
    }
}


void OgldevBackendSetMousePos(uint x, uint y)
{
    switch (sBackendType) {
        case OGLDEV_BACKEND_TYPE_GLUT:
           //GLUTBackendLeaveMainLoop();
            break;
        case OGLDEV_BACKEND_TYPE_GLFW:
            GLFWBackendSetMousePos(x, y);
            break;
        default:
            assert(0);
    }    
}