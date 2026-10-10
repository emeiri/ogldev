/*
    Regression test shim for GLFW (see Testing/README.md).

    Windows\ogldev_vs_2026\Directory.Build.targets puts Testing\shim in front of
    the include path, so <GLFW/glfw3.h> resolves to this file. It includes the
    real header and then routes a few GLFW calls through the test mode of
    Include/regress_hook.h. That way the tutorials themselves contain no testing
    code. Without --test-frames on the command line every wrapper just calls
    the real function.
*/

#include "../../../Include/GLFW/glfw3.h"

#if defined(__cplusplus) && !defined(REGRESS_GLFW_SHIM_H)
#define REGRESS_GLFW_SHIM_H

#include "../../../Include/regress_hook.h"

namespace Regress {

// The fixed 1/60 clock
inline double GlfwGetTime()
{
    return Time(glfwGetTime());
}


// Never full screen
inline GLFWwindow* GlfwCreateWindow(int Width, int Height, const char* pTitle, GLFWmonitor* pMonitor, GLFWwindow* pShare)
{
    return glfwCreateWindow(Width, Height, pTitle, Active() ? NULL : pMonitor, pShare);
}


// No input
inline GLFWkeyfun GlfwSetKeyCallback(GLFWwindow* pWindow, GLFWkeyfun Callback)
{
    return Active() ? NULL : glfwSetKeyCallback(pWindow, Callback);
}


inline GLFWcharfun GlfwSetCharCallback(GLFWwindow* pWindow, GLFWcharfun Callback)
{
    return Active() ? NULL : glfwSetCharCallback(pWindow, Callback);
}


inline GLFWcursorposfun GlfwSetCursorPosCallback(GLFWwindow* pWindow, GLFWcursorposfun Callback)
{
    return Active() ? NULL : glfwSetCursorPosCallback(pWindow, Callback);
}


inline GLFWmousebuttonfun GlfwSetMouseButtonCallback(GLFWwindow* pWindow, GLFWmousebuttonfun Callback)
{
    return Active() ? NULL : glfwSetMouseButtonCallback(pWindow, Callback);
}


inline GLFWscrollfun GlfwSetScrollCallback(GLFWwindow* pWindow, GLFWscrollfun Callback)
{
    return Active() ? NULL : glfwSetScrollCallback(pWindow, Callback);
}

}

// OpenGL only: the end of the frame is the buffer swap. Vulkan projects end
// their frame in VulkanQueue::Present.
#ifdef GL_VERSION_1_1

#include "../../../Include/regress_capture_gl.h"

namespace Regress {

inline void GlfwSwapBuffers(GLFWwindow* pWindow)
{
    if (Active()) {
        if (!GetState().Capture) {
            InitGL();
        }

        glfwGetFramebufferSize(pWindow, &GetState().Width, &GetState().Height);

        if (EndFrame()) {
            glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
        }
    }

    glfwSwapBuffers(pWindow);
}

}

#define glfwSwapBuffers Regress::GlfwSwapBuffers

#endif

#define glfwGetTime                Regress::GlfwGetTime
#define glfwCreateWindow           Regress::GlfwCreateWindow
#define glfwSetKeyCallback         Regress::GlfwSetKeyCallback
#define glfwSetCharCallback        Regress::GlfwSetCharCallback
#define glfwSetCursorPosCallback   Regress::GlfwSetCursorPosCallback
#define glfwSetMouseButtonCallback Regress::GlfwSetMouseButtonCallback
#define glfwSetScrollCallback      Regress::GlfwSetScrollCallback

#endif
