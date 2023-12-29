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

    Tutorial 51 - Quad Tessellation
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"



#include "ogldev_util.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_glfw.h"
#include "ogldev_quad_tess_technique.h"
#include "ogldev_vertex_buffer.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class Tutorial51
{
public:

    Tutorial51()
    {
    }


    virtual ~Tutorial51()
    {
        SAFE_DELETE(m_pGameCamera);
    }


    void Init()
    {
        CreateWindow();

        InitCallbacks();

        InitCamera();

        InitMesh();

        InitShaders();

        InitGUI();

        glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glLineWidth(5.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }


    void Run()
    {
        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderSceneCB();
            RenderGui();            
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }


    void RenderSceneCB()
    {
        m_quadTessTech.SetWVP(m_pGameCamera->GetViewProjMatrix());
        m_quadTessTech.SetLevels(m_outerLevelLeft,
            m_outerLevelBottom,
            m_outerLevelRight,
            m_outerLevelTop,
            m_innerLevelLeftRight,
            m_innerLevelTopBottom);
        m_vertexBuffer.Render();
    }


    void RenderGui()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Terrain Water Demo");

        ImGui::SliderFloat("Outer Left Tessellation Level", &this->m_outerLevelLeft, 0.0f, 50.0f);
        ImGui::SliderFloat ("Outer Bottom Tessellation Level", &this->m_outerLevelBottom, 0.0f, 50.0f);
        ImGui::SliderFloat ("Outer Right Tessellation Level", &this->m_outerLevelRight, 0.0f, 50.0f);
        ImGui::SliderFloat("Outer Top Tessellation Level", &this->m_outerLevelTop, 0.0f, 50.0f);

        ImGui::SliderFloat("Inner Left/Right Tessellation Level", &this->m_innerLevelLeftRight, 0.0f, 50.0f);
        ImGui::SliderFloat("Inner Top/Bottom Tessellation Level", &this->m_innerLevelTopBottom, 0.0f, 50.0f);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void PassiveMouseCB(int x, int y)
    {
    }


#define STEP 0.01f

    void KeyboardCB(uint key, int state)
    {
        bool Handled = true;

        if (state == GLFW_PRESS) {
            switch (key) {
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
                glfwDestroyWindow(window);
                glfwTerminate();
                exit(0);

            case GLFW_KEY_P:
                m_isPaused = !m_isPaused;
                break;

            case GLFW_KEY_Z:
                m_isWireframe = !m_isWireframe;

                if (m_isWireframe) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
                else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
                break;

            default:
                Handled = false;
            }
        }

        if (!Handled) {
            m_pGameCamera->OnKeyboard(key);
        }
    }


    void MouseCB(int button, int action, int x, int y)
    {
    }


private:

    void CreateWindow()
    {
        int major_ver = 0;
        int minor_ver = 0;
        bool is_full_screen = false;
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 51");

        glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    }


    void InitCallbacks()
    {
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
    }


    void InitCamera()
    {
        Vector3f CameraPos = Vector3f(0.0f, 0.0f, -1.0f);
        Vector3f CameraTarget = Vector3f(0.0f, 0.f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        float c = 3.5f;
        OrthoProjInfo orthoProjinfo;
        orthoProjinfo.l = -0.4f * c;
        orthoProjinfo.r = 0.4f * c;
        orthoProjinfo.b = -0.3f * c;
        orthoProjinfo.t = 0.3f * c;
        orthoProjinfo.n = 0.1f;
        orthoProjinfo.f = 100.0f;
        orthoProjinfo.Width = WINDOW_WIDTH;
        orthoProjinfo.Height = WINDOW_HEIGHT;

        m_pGameCamera = new BasicCamera(orthoProjinfo, CameraPos, CameraTarget, Up);
        m_pGameCamera->SetSpeed(0.1f);
    }


    void InitShaders()
    {
        if (!m_quadTessTech.Init()) {
            printf("Error initializing the bezier curve technique\n");
            exit(1);
        }

        m_quadTessTech.Enable();
        m_quadTessTech.SetColor(Vector4f(1.0f, 1.0f, 0.0f, 0.0f));
    }


    void InitMesh()
    {
        int NumVertexElements = 2;
        m_vertexBuffer.Init(m_vertices, NumVertexElements, GL_PATCHES);
    }


    void InitGUI()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        const char* glsl_version = "#version 130";
        ImGui_ImplOpenGL3_Init(glsl_version);
    }


    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    bool m_isPaused = false;
    VertexBuffer m_vertexBuffer;
    QuadTessTechnique m_quadTessTech;
    std::vector<float> m_vertices = { -1.0f, -1.0f,     // Bottom left
                                      1.0f, -1.0f,      // Bottom right
                                      1.0f, 1.0f,       // Top right
                                      -1.0f, 1.0f };    // Top left
    bool m_isWireframe = true;

    float m_outerLevelLeft = 4;
    float m_outerLevelBottom = 4;
    float m_outerLevelRight = 4;
    float m_outerLevelTop = 4;

    float m_innerLevelLeftRight = 4;
    float m_innerLevelTopBottom = 4;
};

Tutorial51* app = NULL;

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    app->KeyboardCB(key, action);
}


static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    app->PassiveMouseCB((int)x, (int)y);
}


static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode)
{
    double x, y;

    glfwGetCursorPos(window, &x, &y);

    app->MouseCB(Button, Action, (int)x, (int)y);
}


int main(int argc, char** argv)
{
    app = new Tutorial51();

    app->Init();

    app->Run();

    delete app;

    return 0;
}
