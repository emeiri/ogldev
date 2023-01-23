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

    Terrain Rendering - demo 3 - midpoint displacement
*/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifdef _WIN64
#include <Windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "ogldev_util.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_glfw.h"

#include "midpoint_disp_terrain.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class TerrainDemo3
{
public:

    TerrainDemo3()
    {
    }

    virtual ~TerrainDemo3()
    {
        SAFE_DELETE(m_pGameCamera);
    }


    void Init()
    {
        CreateWindow_(); // added '_' because of conflict with Windows.h

        InitCallbacks();

        InitCamera();

        InitTerrain();

        InitGUI();
    }


    void Run()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            if (m_showGui) {
                // Start the Dear ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                
                static int Iterations = 100;
                static float MaxHeight = 200.0f;
                static float Roughness = 1.5f;
                 
                ImGui::Begin("Terrain Demo 3");                          // Create a window called "Hello, world!" and append into it.

                ImGui::SliderInt("Iterations", &Iterations, 0, 1000);
                ImGui::SliderFloat("MaxHeight", &MaxHeight, 0.0f, 1000.0f);
                ImGui::SliderFloat("Roughness", &Roughness, 0.0f, 5.0f);

                if (ImGui::Button("Generate")) {
                    m_terrain.Destroy();
                    int Size = 256;
                    float MinHeight = 0.0f;
                    m_terrain.CreateMidpointDisplacement(Size, Roughness, MinHeight, MaxHeight);
                }

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();

                // Rendering
                ImGui::Render();
                int display_w, display_h;
                glfwGetFramebufferSize(window, &display_w, &display_h);
                glViewport(0, 0, display_w, display_h);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            RenderScene();            

            glfwSwapBuffers(window);
        }
    }


    void RenderScene()
    {
        if (!m_showGui) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

      /*  static float foo = 0.0f;
        float R = 400.0f;
        float S = 128.0f;

        Vector3f Pos(S + cosf(foo) * R, 250.0f, S + sinf(foo) * R);
        m_pGameCamera->SetPosition(Pos);

        Vector3f Center(128.0f, 50.0f, 128.0f);
        Vector3f Target = Center - Pos;
        m_pGameCamera->SetTarget(Target);
        m_pGameCamera->SetUp(0.0f, 1.0f, 0.0f);

        foo += 0.001f;*/

        m_terrain.Render(*m_pGameCamera);
    }


    void PassiveMouseCB(int x, int y)
    {
        if (!m_showGui) {
            m_pGameCamera->OnMouse(x, y);
        }
    }

    void KeyboardCB(uint key, int state)
    {
        if (state == GLFW_PRESS) {

            switch (key) {

            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
                glfwDestroyWindow(window);
                glfwTerminate();
                exit(0);

            case GLFW_KEY_C:
                m_pGameCamera->Print();
                break;

            case GLFW_KEY_W:
                m_isWireframe = !m_isWireframe;

                if (m_isWireframe) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }

            case GLFW_KEY_SPACE:
                m_showGui = !m_showGui;
                break;
            }
        }

        m_pGameCamera->OnKeyboard(key);
    }


    void MouseCB(int button, int action, int x, int y)
    {
    }


private:

    void CreateWindow_()
    {
        int major_ver = 0;
        int minor_ver = 0;
        bool is_full_screen = false;
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Terrain Rendering - Demo 3");

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
        Vector3f Pos(500.0f, 300.0f, -150.0f);
        Vector3f Target(0.0f, -0.25f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        float FOV = 45.0f;
        float zNear = 0.1f;
        float zFar = 2000.0f;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

        m_pGameCamera = new BasicCamera(persProjInfo, Pos, Target, Up);
    }
    

    void InitTerrain()
    {
        float WorldScale = 4.0f;
        m_terrain.InitTerrain(WorldScale);

        int Size = 256;
        float Roughness = 1.0f;
        float MinHeight = 0.0f;
        float MaxHeight = 250.0f;

        m_terrain.CreateMidpointDisplacement(Size, Roughness, MinHeight, MaxHeight);
    }


    void InitGUI()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        const char* glsl_version = "#version 130";
        ImGui_ImplOpenGL3_Init(glsl_version);
    }


    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    bool m_isWireframe = false;
    MidpointDispTerrain m_terrain;
    bool m_showGui = false;
};

TerrainDemo3* app = NULL;

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
#ifdef _WIN64
    srand(GetCurrentProcessId());
#else
    srand(getpid());
#endif

    app = new TerrainDemo3();

    app->Init();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    app->Run();

    delete app;

    return 0;
}
