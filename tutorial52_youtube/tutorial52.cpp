/*

        Copyright 2024 Etay Meiri

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

    PBR Textures
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>


#include "ogldev_engine_common.h"
#include "ogldev_util.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_new_lighting.h"
#include "ogldev_glfw.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_world_transform.h"
#include "ogldev_phong_renderer.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class Tutorial43
{
public:

    Tutorial43() : m_albedo(GL_TEXTURE_2D), m_roughness(GL_TEXTURE_2D)
    {
        m_dirLight.WorldDirection = Vector3f(1.0f, -0.15f, 1.0f);
        m_dirLight.DiffuseIntensity = 1.0f;
        m_dirLight.Color = Vector3f(150.0f/256.0f, 150.0f/256.0f, 150.0f/256.0f);
      //  m_dirLight.AmbientIntensity = 0.15f;

        m_pointLights[0].WorldPosition = Vector3f(5.0f, 5.0f, 5.0f);
        //        m_pointLights[0].DiffuseIntensity = 100.0f;

        m_pointLights[1].WorldPosition = Vector3f(-7.0f, 3.0f, 7.0f);
        //        m_pointLights[1].DiffuseIntensity = 100.0f;
    }

    virtual ~Tutorial43()
    {
        SAFE_DELETE(m_pGameCamera);
    }


    void Init()
    {
        CreateWindow();

        InitCallbacks();

        InitCamera();

        InitMesh();

        InitRenderer();

        m_startTime = GetCurrentTimeMillis();
        m_currentTime = m_startTime;
    }


    void Run()
    {
        while (!glfwWindowShouldClose(window)) {
            RenderSceneCB();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }


    void RenderSceneCB()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pGameCamera->OnRender();

        static float foo = 0.0f;
        foo += 0.01f;

        m_pointLights[0].WorldPosition = Vector3f(sinf(foo) * 7, 3.0f, cosf(foo) * 7.0f);
        m_phongRenderer.UpdatePointLightPos(0, m_pointLights[0].WorldPosition);

        m_dirLight.WorldDirection = Vector3f(sinf(foo), -0.5f, cosf(foo));
        m_phongRenderer.UpdateDirLightDir(m_dirLight.WorldDirection);

        m_pMesh->GetPBRMaterial().IsMetal = false;
        m_phongRenderer.Render(m_pMesh);
    }


#define ATTEN_STEP 0.01f

#define ANGLE_STEP 1.0f

    void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
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
            }
        }

        m_pGameCamera->OnKeyboard(key);
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 43");

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
        Vector3f Pos(0.0f, 5.0f, -10.0f);
        Vector3f Target(0.0f, -0.25f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        float FOV = 45.0f;
        float zNear = 0.1f;
        float zFar = 1000.0f;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

        m_pGameCamera = new BasicCamera(persProjInfo, Pos, Target, Up);
    }


    void InitRenderer()
    {
        m_phongRenderer.InitPhongRenderer();
        m_phongRenderer.SetCamera(m_pGameCamera);
        m_phongRenderer.SetDirLight(m_dirLight);
        m_phongRenderer.SetPointLights(ARRAY_SIZE_IN_ELEMENTS(m_pointLights), &m_pointLights[0]);
        m_phongRenderer.SetPBR(true);
    }


    void InitMesh()
    {
        m_pMesh = new SkinnedMesh();
        //        m_pMesh->LoadMesh("../Content/spot/spot_triangulated.obj");
       // m_pMesh->LoadMesh("../Content/dragon.obj");
       // m_pMesh->SetRotation(0.0f, 90.0f, 0.0f);
        m_pMesh->LoadMesh("../Content/box_terrain.obj");

      //  m_albedo.Load("../Content/dry-rocky-ground-bl/dry-rocky-ground_albedo.png");
        m_roughness.Load("../Content/dry-rocky-ground-bl/dry-rocky-ground_roughness.png");
        m_albedo.Load("../Content/textures/rusted_iron/albedo.png");
     //   m_roughness.Load("../Content/textures/rusted_iron/roughness.png");
        m_pMesh->GetPBRMaterial().pAlbedo = &m_albedo;
        m_pMesh->GetPBRMaterial().pRoughness = &m_roughness;

        m_pMesh->SetPBR(true);
    }

    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    PhongRenderer m_phongRenderer;
    SkinnedMesh* m_pMesh = NULL;
    PersProjInfo m_persProjInfo;
    DirectionalLight m_dirLight;
    PointLight m_pointLights[2];
    long long m_startTime = 0;
    long long m_currentTime = 0;
    Texture m_albedo;
    Texture m_roughness;
};

Tutorial43* app = NULL;

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
    app = new Tutorial43();

    app->Init();

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    app->Run();

    delete app;

    return 0;
}
