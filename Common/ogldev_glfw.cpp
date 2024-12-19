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

#ifdef WIN32
#include <Windows.h>
#endif
#include <stdio.h>

#include "ogldev_util.h"
#include "ogldev_glfw.h"
#include "ogldev_glm_camera.h"

static int glMajorVersion = 0;
static int glMinorVersion = 0;

int GetGLMajorVersion()
{
    return glMajorVersion;
}

int GetGLMinorVersion()
{
    return glMinorVersion;
}

int IsGLVersionHigher(int MajorVer, int MinorVer)
{
    return ((glMajorVersion >= MajorVer) && (glMinorVersion >= MinorVer));
}


static void glfw_lib_init()
{
    if (glfwInit() != 1) {
        OGLDEV_ERROR0("Error initializing GLFW");
        exit(1);
    }

    int Major, Minor, Rev;

    glfwGetVersion(&Major, &Minor, &Rev);

    printf("GLFW %d.%d.%d initialized\n", Major, Minor, Rev);
}


static void enable_debug_output()
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
}


    // Must be done after glfw is initialized!
static void init_glew()
{
    glewExperimental = GL_TRUE;
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        OGLDEV_ERROR0((const char*)glewGetErrorString(res));
        exit(1);
    }
}


GLFWwindow* glfw_init(int major_ver, int minor_ver, int width, int height, bool is_full_screen, const char* title)
{
    glfw_lib_init();

    GLFWmonitor* monitor = is_full_screen ? glfwGetPrimaryMonitor() : NULL;

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    if (major_ver > 0) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_ver);
    }

    if (minor_ver > 0) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_ver);
    }

    GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, NULL);

    if (!window) {
        const char* pDesc = NULL;
        int error_code = glfwGetError(&pDesc);

        OGLDEV_ERROR("Error creating window: %s", pDesc);
        exit(1);
    }

    glfwMakeContextCurrent(window);

    // The following functions must be called after the context is made current
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);

    if (major_ver > 0) {
        if (major_ver != glMajorVersion) {
            OGLDEV_ERROR("Requested major version %d is not the same as created version %d", major_ver, glMajorVersion);
            exit(0);
        }
    }

    if (minor_ver > 0) {
        if (minor_ver != glMinorVersion) {
            OGLDEV_ERROR("Requested minor version %d is not the same as created version %d", minor_ver, glMinorVersion);
            exit(0);
        }
    }

    // Must be done after glfw is initialized!
    init_glew();

    enable_debug_output();

    glfwSwapInterval(1);

    return window;
}


#if 0

/* !!!This code was introduced in OpenGL tutorial 55 (GLM Camera).It has been moved to
<ogldev>\Common\ogldev_glfw_camera_handler.cpp because it is now shared by both OpenGL and
Vulkan but this file cannot be compiled in Vulkan because of too much OpenGL incompatibilities.
I'm keeping it here as a reference to match the video. */

bool GLFWCameraHandler(CameraMovement& Movement, int Key, int Action, int Mods)
{
    bool Press = Action != GLFW_RELEASE;

    bool Handled = true;

    switch (Key) {

    case GLFW_KEY_W:
        Movement.Forward = Press;
        break;

    case GLFW_KEY_S:
        Movement.Backward = Press;
        break;

    case GLFW_KEY_A:
        Movement.StrafeLeft = Press;
        break;

    case GLFW_KEY_D:
        Movement.StrafeRight = Press;
        break;

    case GLFW_KEY_PAGE_UP:
        Movement.Up = Press;
        break;

    case GLFW_KEY_PAGE_DOWN:
        Movement.Down = Press;
        break;

    case GLFW_KEY_KP_ADD:
        Movement.Plus = Press;
        break;

    case GLFW_KEY_KP_SUBTRACT:
        Movement.Minus = Press;
        break;

    default:
        Handled = false;
    }

    if (Mods & GLFW_MOD_SHIFT) {
        Movement.FastSpeed = Press;
    }

    return Handled;
}

#endif