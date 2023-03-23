/*

        Copyright 2022 Etay Meiri

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

#include <stdio.h>
#include <stdlib.h>

#include "ogldev_rendering_subsystem_gl.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_glfw.h"
#include "ogldev_basic_glfw_camera.h"

extern BaseRenderingSubsystem* g_pRenderingSubsystem;

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ((RenderingSubsystemGL*)g_pRenderingSubsystem)->OnKeyCallback(window, key, scancode, action, mods);
}


static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    ((RenderingSubsystemGL*)g_pRenderingSubsystem)->OnCursorPosCallback(window, x, y);
}


static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode)
{
    double x, y;

    glfwGetCursorPos(window, &x, &y);

    ((RenderingSubsystemGL*)g_pRenderingSubsystem)->OnMouseButtonCallback(window, Button, Action, Mode);
}


RenderingSubsystemGL::RenderingSubsystemGL(GameCallbacks* pGameCallbacks) : BaseRenderingSubsystem(pGameCallbacks)
{
}


RenderingSubsystemGL::~RenderingSubsystemGL()
{
    SAFE_DELETE(m_pCamera);
}


void RenderingSubsystemGL::Shutdown()
{
    if (m_pWindow) {
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
    }

    delete this;
}

void RenderingSubsystemGL::CreateWindow(int Width, int Height)
{
    m_windowWidth = Width;
    m_windowHeight = Height;
    int major_ver = 0;
    int minor_ver = 0;
    bool is_full_screen = false;
    m_pWindow = glfw_init(major_ver, minor_ver, Width, Height, is_full_screen, "Rendering Subsystem GL demo");

    glfwSetCursorPos(m_pWindow, Width / 2, Height / 2);

    SetDefaultGLState();

    InitCallbacks();
}


void RenderingSubsystemGL::SetDefaultGLState()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}


void RenderingSubsystemGL::Execute()
{
    if (!m_pCamera) {
        printf("%s:%d - camera has not been set\n", __FILE__, __LINE__);
        exit(0);
    }

    long long StartTimeMillis = GetCurrentTimeMillis();

    while (!glfwWindowShouldClose(m_pWindow)) {
        long long CurrentTimeMillis = GetCurrentTimeMillis();
        m_elapsedTimeMillis = CurrentTimeMillis - StartTimeMillis;
        m_pCamera->OnRender();
        m_pGameCallbacks->OnFrame();
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }
}


void RenderingSubsystemGL::ClearWindow()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderingSubsystemGL::InitCallbacks()
{
    glfwSetKeyCallback(m_pWindow, KeyCallback);
    glfwSetCursorPosCallback(m_pWindow, CursorPosCallback);
    glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
}


void RenderingSubsystemGL::OnKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    bool Handled = m_pGameCallbacks->OnKeyboard(key, action);

    if (!Handled) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            Shutdown();
            exit(0);

        default:
            m_pCamera->OnKeyboard(key);
        }
    }
}


void RenderingSubsystemGL::OnCursorPosCallback(GLFWwindow* window, double x, double y)
{
    m_pCamera->OnMouse((int)x, (int)y);
    m_pGameCallbacks->OnMouseMove((int)x, (int)y);
}


void RenderingSubsystemGL::OnMouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode)
{
    m_pGameCallbacks->OnMouseButton(Button, Action, Mode);
}
