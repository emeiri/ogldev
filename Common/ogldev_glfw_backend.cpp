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
#include <GL/glew.h>
#include <GLFW/glfw3.h>



#include "ogldev_util.h"
#include "ogldev_glfw_backend.h"

// Points to the object implementing the ICallbacks interface which was delivered to
// GLUTBackendRun(). All events are forwarded to this object.
static ICallbacks* s_pCallbacks = NULL;

static bool sWithDepth = false;
static bool sWithStencil = false;
static GLFWwindow* s_pWindow = NULL;

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}


static void InitCallbacks()
{
    /*glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(IdleCB);
    glutSpecialFunc(SpecialKeyboardCB);
    glutPassiveMotionFunc(PassiveMouseCB);
    glutMouseFunc(MouseCB);*/
    glfwSetKeyCallback(s_pWindow, KeyCallback);
}

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

void GLFWBackendInit(int argc, char** argv, bool WithDepth, bool WithStencil)
{
    sWithDepth = WithDepth;
    sWithStencil = WithStencil;

    if (glfwInit() != 1) {
        OGLDEV_ERROR("Error initializing GLFW");
        exit(1);
    }
    
    int Major, Minor, Rev;
    
    glfwGetVersion(&Major, &Minor, &Rev);
    
    printf("GLFW %d.%d.%d initialized\n", Major, Minor, Rev);
    
    glfwSetErrorCallback(GLFWErrorCallback);
    
//    glfwWindowHint( GLFW_OPENGL_VERSION_MAJOR, 3 );
  //  glfwOpenWindowHint( GLFW_OPENGL_VERSION_MINOR, 3 );
  //  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    }


void GLFWBackendTerminate()
{
    glfwDestroyWindow(s_pWindow);
    glfwTerminate();
}


bool GLFWBackendCreateWindow(unsigned int Width, unsigned int Height, bool isFullScreen, const char* pTitle)
{
    GLFWmonitor* pMonitor = isFullScreen ? glfwGetPrimaryMonitor() : NULL;

    s_pWindow = glfwCreateWindow(Width, Height, pTitle, pMonitor, NULL);

    if (!s_pWindow) {
        OGLDEV_ERROR("error creating window");
        exit(1);
    }
    
    glfwMakeContextCurrent(s_pWindow);
    
    // Must be done after glfw is initialized!
    glewExperimental = GL_TRUE;
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        OGLDEV_ERROR((const char*)glewGetErrorString(res));
        exit(1);
    }    
    
    return (s_pWindow != NULL);
}

void GLFWBackendRun(ICallbacks* pCallbacks)
{
    if (!pCallbacks) {
        fprintf(stderr, "%s : callbacks not specified!\n", __FUNCTION__);
        return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    if (sWithDepth) {
        glEnable(GL_DEPTH_TEST);
    }

    s_pCallbacks = pCallbacks;
    InitCallbacks();

    while (!glfwWindowShouldClose(s_pWindow)) {
        s_pCallbacks->RenderSceneCB();
        glfwSwapBuffers(s_pWindow);
        glfwPollEvents();
    }
}