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
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "ogldev_util.h"
#include "ogldev_glfw_backend.h"

// Points to the object implementing the ICallbacks interface which was delivered to
// GLUTBackendRun(). All events are forwarded to this object.
static ICallbacks* s_pCallbacks = NULL;

static bool sWithDepth = false;
static bool sWithStencil = false;
static GLFWwindow* s_pWindow = NULL;


static OGLDEV_KEY GLFWKeyToOGLDEVKey(uint Key)
{
    if (Key >= GLFW_KEY_SPACE && Key <= GLFW_KEY_RIGHT_BRACKET) {
        return (OGLDEV_KEY)Key;
    }
    
    switch (Key) {
        case GLFW_KEY_ESCAPE:            
            return OGLDEV_KEY_ESCAPE;
        case GLFW_KEY_ENTER:         
            return OGLDEV_KEY_ENTER;
        case GLFW_KEY_TAB:          
            return OGLDEV_KEY_TAB;
        case GLFW_KEY_BACKSPACE:  
            return OGLDEV_KEY_BACKSPACE;
        case GLFW_KEY_INSERT:         
            return OGLDEV_KEY_INSERT;
        case GLFW_KEY_DELETE:        
            return OGLDEV_KEY_DELETE;
        case GLFW_KEY_RIGHT:         
            return OGLDEV_KEY_RIGHT;
        case GLFW_KEY_LEFT:         
            return OGLDEV_KEY_LEFT;
        case GLFW_KEY_DOWN:        
            return OGLDEV_KEY_DOWN;            
        case GLFW_KEY_UP:         
            return OGLDEV_KEY_UP;
        case GLFW_KEY_PAGE_UP:   
            return OGLDEV_KEY_PAGE_UP;
        case GLFW_KEY_PAGE_DOWN:      
            return OGLDEV_KEY_PAGE_DOWN;
        case GLFW_KEY_HOME:    
            return OGLDEV_KEY_HOME;
        case GLFW_KEY_END:     
            return OGLDEV_KEY_END;
        case GLFW_KEY_F1:        
            return OGLDEV_KEY_F1;
        case GLFW_KEY_F2:        
            return OGLDEV_KEY_F2;
        case GLFW_KEY_F3:       
            return OGLDEV_KEY_F3;
        case GLFW_KEY_F4:   
            return OGLDEV_KEY_F4;
        case GLFW_KEY_F5:      
            return OGLDEV_KEY_F5;
        case GLFW_KEY_F6:     
            return OGLDEV_KEY_F6;
        case GLFW_KEY_F7:     
            return OGLDEV_KEY_F7;
        case GLFW_KEY_F8:     
            return OGLDEV_KEY_F8;
        case GLFW_KEY_F9:     
            return OGLDEV_KEY_F9;
        case GLFW_KEY_F10:    
            return OGLDEV_KEY_F10;
        case GLFW_KEY_F11:   
            return OGLDEV_KEY_F11;
        case GLFW_KEY_F12:    
            return OGLDEV_KEY_F12;
        default:
            OGLDEV_ERROR("Unimplemented OGLDEV key");
    }
    
    return OGLDEV_KEY_UNDEFINED;
}

static OGLDEV_MOUSE GLFWMouseToOGLDEVMouse(uint Button)
{
	switch (Button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		return OGLDEV_MOUSE_BUTTON_LEFT;
	case GLFW_MOUSE_BUTTON_RIGHT:
		return OGLDEV_MOUSE_BUTTON_RIGHT;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		return OGLDEV_MOUSE_BUTTON_MIDDLE;
	default:
		OGLDEV_ERROR("Unimplemented OGLDEV mouse button");
	}

	return OGLDEV_MOUSE_UNDEFINED;
}

static void KeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{   
    OGLDEV_KEY OgldevKey = GLFWKeyToOGLDEVKey(key);   
    OGLDEV_KEY_STATE OgldevKeyState = (action == GLFW_PRESS) ? OGLDEV_KEY_STATE_PRESS : OGLDEV_KEY_STATE_RELEASE;
    s_pCallbacks->KeyboardCB(OgldevKey, OgldevKeyState);
}


static void CursorPosCallback(GLFWwindow* pWindow, double x, double y)
{
    s_pCallbacks->PassiveMouseCB((int)x, (int)y);
}


static void MouseButtonCallback(GLFWwindow* pWindow, int Button, int Action, int Mode)
{
    OGLDEV_MOUSE OgldevMouse = GLFWMouseToOGLDEVMouse(Button);

    OGLDEV_KEY_STATE State = (Action == GLFW_PRESS) ? OGLDEV_KEY_STATE_PRESS : OGLDEV_KEY_STATE_RELEASE;

    double x, y;

    glfwGetCursorPos(pWindow, &x, &y);

    s_pCallbacks->MouseCB(OgldevMouse, State, (int)x, (int)y);
}

static void InitCallbacks()
{
    glfwSetKeyCallback(s_pWindow, KeyCallback);
    glfwSetCursorPosCallback(s_pWindow, CursorPosCallback);
    glfwSetMouseButtonCallback(s_pWindow, MouseButtonCallback);
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
    exit(0);
}


void GLFWBackendInit(int argc, char** argv, bool WithDepth, bool WithStencil)
{
    sWithDepth = WithDepth;
    sWithStencil = WithStencil;

    glfwSetErrorCallback(GLFWErrorCallback);    
    
    if (glfwInit() != 1) {
        OGLDEV_ERROR("Error initializing GLFW");
        exit(1);
    }
    
    int Major, Minor, Rev;
    
    glfwGetVersion(&Major, &Minor, &Rev);
    
    printf("GLFW %d.%d.%d initialized\n", Major, Minor, Rev);       
}


void GLFWBackendTerminate()
{
    glfwDestroyWindow(s_pWindow);
    glfwTerminate();
}


bool GLFWBackendCreateWindow(uint Width, uint Height, bool isFullScreen, const char* pTitle)
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
        OGLDEV_ERROR("callbacks not specified");
        exit(1);
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


void GLFWBackendSwapBuffers()
{
    // Nothing to do here
}


void GLFWBackendLeaveMainLoop()
{
    glfwSetWindowShouldClose(s_pWindow, 1);
}


void GLFWBackendSetMousePos(uint x, uint y)
{
    glfwSetCursorPos(s_pWindow, (double)x, (double)y);
}