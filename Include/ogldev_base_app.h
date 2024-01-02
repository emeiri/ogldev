/*
        Copyright 2023 Etay Meiri

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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


#include <GL/glew.h>
#include "ogldev_glfw.h"
#include "ogldev_basic_glfw_camera.h"

class OgldevBaseApp
{
public:
    virtual bool KeyboardCB(uint key, int state);

    virtual void MouseCB(int button, int action, int x, int y);

    virtual void PassiveMouseCB(int x, int y);

    void Run();

protected:
    OgldevBaseApp();

    ~OgldevBaseApp();

    void InitBaseApp();

    void DefaultCreateWindow(int WindowWidth, int WindowHeight, const char* pWindowName);

    void DefaultCreateCameraPers();

    void DefaultInitGUI();

    void DefaultInitCallbacks();   

    virtual void RenderSceneCB() = 0;

    GLFWwindow* m_pWindow = NULL;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    BasicCamera* m_pGameCamera = NULL;
    bool m_isPaused = false;
    bool m_isWireframe = false;
};