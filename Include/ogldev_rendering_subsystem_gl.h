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


#ifndef OGLDEV_RENDERING_SUBSYSTEM_GL_H
#define OGLDEV_RENDERING_SUBSYSTEM_GL_H

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "ogldev_rendering_subsystem.h"

class RenderingSubsystemGL : public BaseRenderingSubsystem
{
 public:
    RenderingSubsystemGL();

    ~RenderingSubsystemGL();

    virtual void InitRenderingSubsystem(GameCallbacks* pGameCallbacks);

    virtual void Shutdown();

    virtual void CreateWindow(uint Width, uint Height);

    virtual void Execute();

    void OnKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void OnCursorPosCallback(GLFWwindow* window, double x, double y);
    void OnMouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);

 private:

    void InitCallbacks();

    void SetDefaultGLState();

    GameCallbacks* m_pGameCallbacks = NULL;
    GLFWwindow* m_pWindow = NULL;
};

#endif
