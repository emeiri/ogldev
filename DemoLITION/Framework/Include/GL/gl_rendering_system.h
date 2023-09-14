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


#pragma once

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "Int/base_rendering_system.h"
#include "gl_forward_renderer.h"
#include "GL/gl_scene.h"

class RenderingSystemGL : public BaseRenderingSystem
{
 public:
    RenderingSystemGL(GameCallbacks* pGameCallbacks, bool LoadBasicShapes);

    ~RenderingSystemGL();

    virtual void Shutdown();

    virtual Scene* CreateEmptyScene();

    virtual DemolitionModel* LoadModelInternal(const std::string& Filename);

    virtual void Execute();

    void OnKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    
    void OnCursorPosCallback(GLFWwindow* window, double x, double y);
    
    void OnMouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);

 protected:
     virtual void CreateWindowInternal();

     virtual void SetCamera(BasicCamera* pCamera) { m_forwardRenderer.SetCamera(pCamera); }

 private:

    void InitCallbacks();

    void SetDefaultGLState();

    GLFWwindow* m_pWindow = NULL;
    ForwardRenderer m_forwardRenderer;
};

