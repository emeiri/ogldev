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

#include "imgui.h"

#include "ogldev_basic_glfw_camera.h"
#include "ogldev_glfw.h"
#include "ogldev_cubemap_texture.h"
#include "GL/gl_rendering_system.h"
#include "GL/gl_grid.h"
#include "GL/gl_model.h"

#define NUM_TEXTURES 1024

extern CoreRenderingSystem* g_pRenderingSystem;

static GLenum WrapModeToGLWrapMode[NUM_WRAP_MODE] = {
    GL_CLAMP_TO_EDGE,
    GL_MIRRORED_REPEAT,
    GL_REPEAT,
    GL_CLAMP_TO_BORDER
};


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


void RenderingSystemGL::ReloadShaders()
{
    m_forwardRenderer.ReloadShaders();
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


CoreModel* RenderingSystemGL::LoadModelInternal(const std::string& Filename, const ModelLoadFlags& Flags)
{
    GLModel* pModel = new GLModel(this);

    if (!pModel->LoadAssimpModel(Filename, Flags)) {
        delete pModel;
        pModel = NULL;
    }

    return pModel;
}


CoreModel* RenderingSystemGL::LoadMeshInternal(const std::string& Filename)
{
    GLModel* pModel = new GLModel(this);

    if (!pModel->LoadMesh(Filename)) {
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


int RenderingSystemGL::LoadTexture2D(const std::string& Filename, TextureConfig* pConfig)
{
    if (m_numTextures == m_textures.size()) {
        printf("%s:%d: out of texture space\n", __FILE__, __LINE__);
        exit(0);
    }

    GLTextureConfig TexConfig;

    if (pConfig) {
        TexConfig.m_wrapMode = WrapModeToGLWrapMode[pConfig->m_wrapMode];
    }

    Texture* pTexture = new Texture(GL_TEXTURE_2D, Filename, &TexConfig);
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

    const char* pExt = strrchr(Filename.c_str(), '.');

    bool IsKTX = pExt && !strcmp(pExt, ".ktx");

    BaseCubmapTexture* pTexture = NULL;
    
    if (IsKTX) {
        pTexture = new CubemapTexture;
        ((CubemapTexture*)pTexture)->LoadKTX(Filename);
    } else {
        pTexture = new CubemapEctTexture(Filename);
        pTexture->Load();
    }

    m_textures[m_numTextures] = pTexture;
    int ret = m_numTextures;
    m_numTextures++;

    printf("Cubemap texture '%s' loaded, handle %d\n", Filename.c_str(), ret);

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

    double StartTime = glfwGetTime();
    double CurTime = glfwGetTime();
    double PrevTime = CurTime;

    // Skip a few frames to avoid instability with zero or too large delta time at the start of the application
    for (int i = 0; i < 3; i++) {
        CurTime = glfwGetTime();
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }

    while (!glfwWindowShouldClose(m_pWindow)) {
        PrevTime = CurTime;
        CurTime = glfwGetTime();
        double DeltaTime = CurTime - PrevTime;
        double TotalRuntime = CurTime - StartTime;

        m_elapsedTime = CurTime - StartTime;
        m_pCamera->Update((float)DeltaTime);
        if (m_pScene) {
            m_forwardRenderer.Render(m_pWindow, (GLScene*)m_pScene, m_pGameCallbacks, TotalRuntime, DeltaTime);
        } else {
            printf("Warning! no scene is set in the rendering subsystem\n");
        }
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }
}


void RenderingSystemGL::ImGuiTextureWindow(const char* pTitle)
{
    ImGui::Begin(pTitle, nullptr);

    const ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    const ImVec2 vMax = ImGui::GetWindowContentRegionMax();

    GLuint TexID = m_forwardRenderer.GetSSAOTextureHandle();

    ImGui::Image((void*)(intptr_t)TexID,
                 ImVec2(vMax.x - vMin.x, vMax.y - vMin.y),
                 ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)
    );

    ImGui::End();
}


void RenderingSystemGL::InitCallbacks()
{
    glfwSetKeyCallback(m_pWindow, KeyCallback);
    glfwSetCursorPosCallback(m_pWindow, CursorPosCallback);
    glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
}


void RenderingSystemGL::OnKeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{
    if (m_pScene) {
        if (action != GLFW_REPEAT) {
          //  printf("key %d action %d\n", key, action);

            InputState& is = m_pScene->GetInputStateInternal();

            if (action == GLFW_RELEASE) {
                is.Keys[key].Released = is.Keys[key].Pressed;
            //    printf("released reset to %lld\n", is.Keys[key].Released);
            } else {
                is.Keys[key].Pressed++;
           //     printf("pressed incremented to %lld\n", is.Keys[key].Pressed);
            }
        }
    }

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
}

// Code dupplication with two other places
static bool IsMouseControlledByImGUI()
{
    ImGuiIO& io = ImGui::GetIO();

    bool ret = io.WantCaptureMouse;

    return ret;
}


void RenderingSystemGL::OnMouseButtonCallback(GLFWwindow* pWindow, int Button, int Action, int Mode)
{
    int MousePosX = 0, MousePosY = 0;
    GetMousePos(pWindow, MousePosX, MousePosY);
    bool HandledByGame = m_pGameCallbacks->OnMouseButton(Button, Action, Mode, MousePosX, MousePosY);

    if (!HandledByGame && !IsMouseControlledByImGUI()) {
        m_pCamera->HandleMouseButton(Button, Action, Mode);
    }
}


void RenderingSystemGL::GetMousePos(void* pWindow, int& x, int& y)
{
    GLFWwindow* pGLFWWindow = (GLFWwindow*)pWindow;
    double MousePosX = 0.0, MousePosY = 0.0;
    glfwGetCursorPos(pGLFWWindow, &MousePosX, &MousePosY);
    x = (int)MousePosX;
    y = (int)MousePosY;
}

struct TerrainVertex {
    glm::vec3 position;  // Maps to layout(location = 0) -> (col, 0.0f, row)
    glm::vec2 texCoords; // Maps to layout(location = 1) -> (u, v)
};


void* RenderingSystemGL::CreateTerrainGrid(int width, int height)
{
    std::vector<TerrainVertex> vertices;
    std::vector<u32> indices;

    TerrainGrid* pGrid = new TerrainGrid();

    // 1. Generate Vertex Buffer Data
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            TerrainVertex vertex;

            // X and Z represent meters. Y is kept flat for the shader to displace.
            vertex.position.x = (float)(col);
            vertex.position.y = 0.0f;
            vertex.position.z = (float)(row);

            // Normalize UV coordinates linearly from 0.0 to 1.0
            vertex.texCoords.x = (float)(col) / (float)(width - 1);
            vertex.texCoords.y = (float)(row) / (float)(height - 1);

            vertices.push_back(vertex);
        }
    }

    // 2. Generate Index Buffer Data (Stitching quads together into triangles)
    for (int row = 0; row < height - 1; ++row) {
        for (int col = 0; col < width - 1; ++col) {
            // Find index pointers for the 4 corners of the current quad
            u32 topLeft = row * width + col;
            u32 topRight = topLeft + 1;
            u32 bottomLeft = (row + 1) * width + col;
            u32 bottomRight = bottomLeft + 1;

            // Triangle 1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Triangle 2
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    pGrid->m_indexCount = (u32)(indices.size());

    // 3. Create GPU buffers utilizing modern OpenGL 4.6 DSA layout structures
    glCreateVertexArrays(1, &pGrid->m_vao);
    glCreateBuffers(1, &pGrid->m_vbo);
    glCreateBuffers(1, &pGrid->m_ebo);

    // Allocate immutable memory banks directly onto the graphics card hardware
    glNamedBufferStorage(pGrid->m_vbo, vertices.size() * sizeof(TerrainVertex), vertices.data(), 0);
    glNamedBufferStorage(pGrid->m_ebo, indices.size() * sizeof(u32), indices.data(), 0);

    // 4. Set up VAO bindings and formatting descriptions natively without binding the state machine
    glVertexArrayVertexBuffer(pGrid->m_vao, 0, pGrid->m_vbo, 0, sizeof(TerrainVertex));
    glVertexArrayElementBuffer(pGrid->m_vao, pGrid->m_ebo);

    // Attribute 0: Position Vector (vec3)
    glEnableVertexArrayAttrib(pGrid->m_vao, 0);
    glVertexArrayAttribFormat(pGrid->m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(TerrainVertex, position));
    glVertexArrayAttribBinding(pGrid->m_vao, 0, 0);

    // Attribute 1: Texture UV Coordinates (vec2)
    glEnableVertexArrayAttrib(pGrid->m_vao, 1);
    glVertexArrayAttribFormat(pGrid->m_vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(TerrainVertex, texCoords));
    glVertexArrayAttribBinding(pGrid->m_vao, 1, 0);

    return pGrid;
}


void TerrainGrid::Render()
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}