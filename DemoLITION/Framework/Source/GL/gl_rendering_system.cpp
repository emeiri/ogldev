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

#include "ogldev_basic_glfw_camera.h"
#include "ogldev_glfw.h"
#include "ogldev_cubemap_texture.h"
#include "GL/gl_rendering_system.h"
#include "GL/gl_grid.h"

#define NUM_TEXTURES 1024

extern CoreRenderingSystem* g_pRenderingSystem;

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ((RenderingSystemGL*)g_pRenderingSystem)->OnKeyCallback(window, key, scancode, action, mods);
}


static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    ((RenderingSystemGL*)g_pRenderingSystem)->OnCursorPosCallback(window, x, y);
}


static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode)
{
    double x, y;

    glfwGetCursorPos(window, &x, &y);

    ((RenderingSystemGL*)g_pRenderingSystem)->OnMouseButtonCallback(window, Button, Action, Mode);
}


RenderingSystemGL::RenderingSystemGL(GameCallbacks* pGameCallbacks, bool LoadBasicShapes) : CoreRenderingSystem(pGameCallbacks, LoadBasicShapes)
{
    m_textures.resize(NUM_TEXTURES, 0);
}


RenderingSystemGL::~RenderingSystemGL()
{
}


void RenderingSystemGL::Shutdown()
{
    if (m_pWindow) {
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
    }

    delete this;
}

void* RenderingSystemGL::CreateWindowInternal(const char* pWindowName)
{
    int major_ver = 0;
    int minor_ver = 0;
    bool is_full_screen = false;
    m_pWindow = glfw_init(major_ver, minor_ver, m_windowWidth, m_windowHeight, is_full_screen, pWindowName);

    glfwSetCursorPos(m_pWindow, m_windowWidth / 2, m_windowHeight / 2);

    SetDefaultGLState();

    InitCallbacks();

    m_forwardRenderer.InitForwardRenderer(this);

    return m_pWindow;
}


Scene* RenderingSystemGL::CreateEmptyScene()
{
    return new GLScene(this);
}


CoreModel* RenderingSystemGL::LoadModelInternal(const std::string& Filename)
{
    CoreModel* pModel = new CoreModel(this);

    if (!pModel->LoadAssimpModel(Filename)) {
        delete pModel;
        pModel = NULL;
    }

    return pModel;
}


Grid* RenderingSystemGL::CreateGridInternal(int Width, int Depth)
{
    GLGrid* pGrid = new GLGrid(Width, Depth);
    return pGrid;
}


int RenderingSystemGL::LoadTexture2D(const std::string& Filename)
{
    if (m_numTextures == m_textures.size()) {
        printf("%s:%d: out of texture space\n", __FILE__, __LINE__);
        exit(0);
    }

    Texture* pTexture = new Texture(GL_TEXTURE_2D, Filename);
    pTexture->Load();

    m_textures[m_numTextures] = pTexture;
    int ret = m_numTextures;
    m_numTextures++;

    printf("2D texture '%s' loaded, handle %d\n", Filename.c_str(), ret);

    return ret;
}


int RenderingSystemGL::LoadCubemapTexture(const std::string& Filename)
{
    if (m_numTextures == m_textures.size()) {
        printf("%s:%d: out of texture space\n", __FILE__, __LINE__);
        exit(0);
    }

    CubemapEctTexture* pTexture = new CubemapEctTexture(Filename);
    pTexture->Load();

    m_textures[m_numTextures] = pTexture;
    int ret = m_numTextures;
    m_numTextures++;

    printf("Equirectangular texture '%s' loaded, handle %d\n", Filename.c_str(), ret);

    return ret;
}


BaseTexture* RenderingSystemGL::GetTexture(int TextureHandle)
{
    if (TextureHandle < 0) {
        printf("%s:%d: invalid texture handle %d\n", __FILE__, __LINE__, TextureHandle);
        exit(0);
    }

    if (TextureHandle >= m_numTextures) {
        printf("%s:%d: invalid texture handle %d\n", __FILE__, __LINE__, TextureHandle);
        exit(0);
    }

    BaseTexture* pTexture = m_textures[TextureHandle];

    if (!pTexture) {
        printf("%s:%d: invalid texture handle %d\n", __FILE__, __LINE__, TextureHandle);
        exit(0);
    }

    return pTexture;
}


void RenderingSystemGL::SetDefaultGLState()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}


void RenderingSystemGL::Execute()
{
    if (!m_pCamera) {
        printf("%s:%d - camera has not been set\n", __FILE__, __LINE__);
        return;
    }

    long long StartTimeMillis = GetCurrentTimeMillis();
    long long CurTimeMillis = StartTimeMillis;

    while (!glfwWindowShouldClose(m_pWindow)) {
        long long PrevTimeMillis = CurTimeMillis;
        CurTimeMillis = GetCurrentTimeMillis();
        long long DeltaTimeMillis = CurTimeMillis - PrevTimeMillis;
        long long TotalRuntimeMillis = CurTimeMillis - StartTimeMillis;
       // printf("Total runtime %I64d delta %I64d\n", TotalRuntimeMillis, DeltaTimeMillis);
        m_elapsedTimeMillis = CurTimeMillis - StartTimeMillis;
        m_pCamera->Update((float)DeltaTimeMillis / 1000.0f);
        if (m_pScene) {
            m_forwardRenderer.Render(m_pWindow, (GLScene*)m_pScene, m_pGameCallbacks, TotalRuntimeMillis, DeltaTimeMillis);
        } else {
            printf("Warning! no scene is set in the rendering subsystem\n");
        }
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }
}


void RenderingSystemGL::InitCallbacks()
{
    glfwSetKeyCallback(m_pWindow, KeyCallback);
    glfwSetCursorPosCallback(m_pWindow, CursorPosCallback);
    glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
}


void RenderingSystemGL::OnKeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{
    bool HandledByGame = m_pGameCallbacks->OnKeyboard(key, action);
    bool HandledByCamera = false;

    if (!HandledByGame) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            Shutdown();
            exit(0);

        case GLFW_KEY_C:
            if (action == GLFW_PRESS) {
                m_pCamera->Print();
            }
            break;

        default:
            HandledByCamera = GLFWCameraHandler(m_pCamera->m_movement, key, action, mods);
        }
    }
}


void RenderingSystemGL::OnCursorPosCallback(GLFWwindow* pWindow, double x, double y)
{
    bool HandledByGame = m_pGameCallbacks->OnMouseMove((int)x, (int)y);

    int WindowWidth, WindowHeight;
    glfwGetWindowSize(m_pWindow, &WindowWidth, &WindowHeight);

    m_pCamera->SetMousePos((float)x, (float)y);

 /*   if (HandledByGame) {
        m_pCamera->UpdateMousePosSilent((int)x, (int)y);
    } else {
        m_pCamera->OnMouse((int)x, (int)y);
    }*/
}


void RenderingSystemGL::OnMouseButtonCallback(GLFWwindow* pWindow, int Button, int Action, int Mode)
{
    int MousePosX = 0, MousePosY = 0;
    GetMousePos(pWindow, MousePosX, MousePosY);
    m_pGameCallbacks->OnMouseButton(Button, Action, Mode, MousePosX, MousePosY);

    m_pCamera->HandleMouseButton(Button, Action, Mode);
}


void RenderingSystemGL::GetMousePos(void* pWindow, int& x, int& y)
{
    GLFWwindow* pGLFWWindow = (GLFWwindow*)pWindow;
    double MousePosX = 0.0, MousePosY = 0.0;
    glfwGetCursorPos(pGLFWWindow, &MousePosX, &MousePosY);
    x = (int)MousePosX;
    y = (int)MousePosY;
}
