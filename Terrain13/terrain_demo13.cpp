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

    Terrain Rendering - demo 13 - Water
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

#include "demo_config.h"
#include "texture_config.h"
#include "midpoint_disp_terrain.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);

static int g_seed = 0;

extern int gShowPoints;


class TerrainDemo9
{
public:

    TerrainDemo9()
    {
        m_waveParams[0].WaveLen = 1.0f;
        m_waveParams[0].Speed = 1.0f;
        m_waveParams[0].Amp = 1.0f;

        m_waveParams[1].WaveLen = 1.0f;
        m_waveParams[1].Speed = 1.0f;
        m_waveParams[1].Amp = 1.0f;

        m_waveParams[2].WaveLen = 1.0f;
        m_waveParams[2].Speed = 1.0f;
        m_waveParams[2].Amp = 1.0f;

        m_waveParams[3].WaveLen = 1.0f;
        m_waveParams[3].Speed = 1.0f;
        m_waveParams[3].Amp = 1.0f;

        m_terrain.SetWaveParam(0, m_waveParams[0]);
        m_terrain.SetWaveParam(1, m_waveParams[1]);
        m_terrain.SetWaveParam(2, m_waveParams[2]);
        m_terrain.SetWaveParam(3, m_waveParams[3]);
    }


    virtual ~TerrainDemo9()
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
                                 
                ImGui::Begin("Terrain Water Demo"); 

                ImGui::SliderFloat("Water height", &this->m_waterHeight, 0.0f, m_maxHeight);
                m_terrain.SetWaterHeight(m_waterHeight);

                ImGui::SliderFloat("Wave0: len", &this->m_waveParams[0].WaveLen, 0.0f, 500.0);
                ImGui::SliderFloat("Wave0: speed", &this->m_waveParams[0].Speed, 0.01f, 50.0);
                ImGui::SliderFloat("Wave0: amplitude", &this->m_waveParams[0].Amp, 0.1f, 20.0);

                ImGui::SliderFloat("Wave1: len", &this->m_waveParams[1].WaveLen, 0.0f, 500.0);
                ImGui::SliderFloat("Wave1: speed", &this->m_waveParams[1].Speed, 0.01f, 50.0);
                ImGui::SliderFloat("Wave1: amplitude", &this->m_waveParams[1].Amp, 0.1f, 20.0);

                ImGui::SliderFloat("Wave2: len", &this->m_waveParams[2].WaveLen, 0.0f, 500.0);
                ImGui::SliderFloat("Wave2: speed", &this->m_waveParams[2].Speed, 0.01f, 50.0);
                ImGui::SliderFloat("Wave2: amplitude", &this->m_waveParams[2].Amp, 0.1f, 20.0);

                ImGui::SliderFloat("Wave3: len", &this->m_waveParams[3].WaveLen, 0.0f, 500.0);
                ImGui::SliderFloat("Wave3: speed", &this->m_waveParams[3].Speed, 0.01f, 50.0);
                ImGui::SliderFloat("Wave3: amplitude", &this->m_waveParams[3].Amp, 0.1f, 20.0);

                m_terrain.SetWaveParam(0, m_waveParams[0]);
                m_terrain.SetWaveParam(1, m_waveParams[1]);
                m_terrain.SetWaveParam(2, m_waveParams[2]);
                m_terrain.SetWaveParam(3, m_waveParams[3]);

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

        static float foo = 0.0f;
        foo += 0.002f;

      /*  float S = (float)m_terrainSize;
        float R = 2.5f * S;

        Vector3f Pos(S + cosf(foo) * R, m_maxHeight + 250.0f, S + sinf(foo) * R);
        m_pGameCamera->SetPosition(Pos);

        Vector3f Center(S, Pos.y * 0.50f, S);
        Vector3f Target = Center - Pos;
        m_pGameCamera->SetTarget(Target);
        m_pGameCamera->SetUp(0.0f, 1.0f, 0.0f);*/

        float y = min(-0.4f, cosf(foo));
        Vector3f LightDir(sinf(foo * 5.0f), y, cosf(foo * 5.0f));

        Vector3f LightDir2(1.0f, -1.0f, 0.0f);

        m_terrain.SetLightDir(LightDir);

        m_terrain.Render(*m_pGameCamera);
    }


    void PassiveMouseCB(int x, int y)
    {
        if (!m_showGui && !m_isPaused) {
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

            case GLFW_KEY_B:
                m_constrainCamera = !m_constrainCamera;
                printf("constrain %d\n", m_constrainCamera);
                break;

            case GLFW_KEY_C:
                m_pGameCamera->Print();
                break;

            case GLFW_KEY_Z:
                m_isWireframe = !m_isWireframe;

                if (m_isWireframe) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
                break;

            case GLFW_KEY_P:
                m_isPaused = !m_isPaused;
                break;

            case GLFW_KEY_SPACE:
                m_showGui = !m_showGui;
                break;

            case GLFW_KEY_0:
                gShowPoints = 0;
                break;

            case GLFW_KEY_1:
                gShowPoints = 1;
                break;

            case GLFW_KEY_2:
                gShowPoints = 2;
                break;

            case GLFW_KEY_F:
                m_waterHeight += 1.0f;
                m_terrain.SetWaterHeight(m_waterHeight);
                break;

            case GLFW_KEY_V:
                m_waterHeight -= 1.0f;
                m_terrain.SetWaterHeight(m_waterHeight);
                break;
            }
        }

        bool CameraChangedPos = m_pGameCamera->OnKeyboard(key);

        if (m_constrainCamera && CameraChangedPos) {
            ConstrainCameraToTerrain();
        }
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Terrain Rendering - Simple Water");

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
        Vector3f Pos(50.0f, 220.0f, 120.0f);
        Vector3f Target(0.0f, -0.15f, -1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        float FOV = 45.0f;
        float zNear = 1.0f;
        float zFar = Z_FAR;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

        m_pGameCamera = new BasicCamera(persProjInfo, Pos, Target, Up);
    }
    

    void InitTerrain()
    {
        float WorldScale = 4.0f;
        float TextureScale = 16.0f;
        std::vector<string> TextureFilenames;
        TextureFilenames.push_back("../Content/textures/rocky_trail_02_diff_1k.jpg");
        TextureFilenames.push_back("../Content/textures/coast_sand_rocks_02_diff_2k.jpg");        
        TextureFilenames.push_back("../Content/textures/brown_mud_leaves_01_diff_2k.jpg");
        TextureFilenames.push_back("../Content/textures/water.png");

        m_terrain.InitTerrain(WorldScale, TextureScale, TextureFilenames);

        m_terrain.CreateMidpointDisplacement(m_terrainSize, m_patchSize, m_roughness, m_minHeight, m_maxHeight);

        Vector3f LightDir(0.0f, -1.0f, 0.0f);

        m_terrain.SetLightDir(LightDir);

        m_terrain.SetWaterHeight(m_waterHeight);
    }


    void InitGUI()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        const char* glsl_version = "#version 130";
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void ConstrainCameraToTerrain()
    {
        Vector3f NewCameraPos = m_terrain.ConstrainCameraPosToTerrain(m_pGameCamera->GetPos());

        m_pGameCamera->SetPosition(NewCameraPos);
    }


    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    bool m_isWireframe = false;
    MidpointDispTerrain m_terrain;
    bool m_showGui = false;
    bool m_isPaused = false;
    int m_terrainSize = 1025;
    float m_roughness = 1.0f;
    float m_minHeight = 0.0f;
    float m_maxHeight = 450.0f;
    int m_patchSize = 33;
    float m_counter = 0.0f;
    bool m_constrainCamera = false;	
    float m_waterHeight = 200.0f;
    WaveParam m_waveParams[MAX_WAVES];
};

TerrainDemo9* app = NULL;

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
    g_seed = GetCurrentProcessId();    
#else
    g_seed = getpid();
#endif
    printf("random seed %d\n", g_seed);

    srand(g_seed);

    app = new TerrainDemo9();

    app->Init();

    glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    app->Run();

    delete app;

    return 0;
}
