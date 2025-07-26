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

#include "Int/core_rendering_system.h"
#include "gl_forward_renderer.h"
#include "GL/gl_scene.h"

class RenderingSystemGL : public CoreRenderingSystem
{
 public:
    RenderingSystemGL(GameCallbacks* pGameCallbacks, bool LoadBasicShapes);

    ~RenderingSystemGL();

    virtual void Shutdown();

    virtual void ReloadShaders();

    virtual Scene* CreateEmptyScene();

    virtual CoreModel* LoadModelInternal(const std::string& Filename);

    virtual CoreModel* LoadMeshInternal(const std::string& Filename);

    virtual Grid* CreateGridInternal(int Width, int Depth);

    virtual int LoadTexture2D(const std::string& Filename);

    virtual int LoadCubemapTexture(const std::string& Filename);

    virtual void Execute();

    virtual void ImGuiTextureWindow(const char* pTitle);

    BaseTexture* GetTexture(int TextureHandle);

    void OnKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    
    void OnCursorPosCallback(GLFWwindow* window, double x, double y);
    
    void OnMouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);

    void GetMousePos(void* pWindow, int& x, int& y);    

 protected:
     virtual void* CreateWindowInternal(const char* pWindowName);

     virtual void SetCamera(GLMCameraFirstPerson* pCamera) { m_forwardRenderer.SetCamera(pCamera); }

 private:

    void InitCallbacks();

    void SetDefaultGLState();    

    GLFWwindow* m_pWindow = NULL;
    ForwardRenderer m_forwardRenderer;
    std::vector<BaseTexture*> m_textures;
    int m_numTextures = 0;
};

