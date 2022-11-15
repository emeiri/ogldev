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

    Physically Based Rendering #1 (intro)
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

    Tutorial43()
    {
        m_dirLight.WorldDirection = Vector3f(1.0f, -0.15f, 1.0f);
        m_dirLight.DiffuseIntensity = 4.0f;
      //  m_dirLight.AmbientIntensity = 0.15f;

        m_pointLights[0].WorldPosition = Vector3f(5.0f, 5.0f, 5.0f);
        //        m_pointLights[0].DiffuseIntensity = 100.0f;

        m_pointLights[1].WorldPosition = Vector3f(-7.0f, 3.0f, 7.0f);
        //        m_pointLights[1].DiffuseIntensity = 100.0f;

        float metalRough = 0.43f;

        // Gold
        m_meshData[0] = { Vector3f(-20.0f, 0.0f, 75.0f), Vector3f(1, 0.71f, 0.29f) };
        // Copper
        m_meshData[1] = { Vector3f(-10.0f, 0.0f, 75.0f), Vector3f(0.95f, 0.64f, 0.54f) };
        // Aluminum
        m_meshData[2] = { Vector3f(-0.0f, 0.0f, 75.0f), Vector3f(0.91f, 0.92f, 0.92f) };
        // Titanium
        m_meshData[3] = { Vector3f(10.0f, 0.0f, 75.0f), Vector3f(0.542f, 0.497f, 0.449f) };
        // Silver
        m_meshData[4] = { Vector3f(20.0f, 0.0f, 75.0f), Vector3f(0.95f, 0.93f, 0.88f) };
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

        for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_meshData) ; i++) {
            m_pMesh->SetPosition(m_meshData[i].Pos);
            m_pMesh->GetPBRMaterial().Roughness = 0.43f;
            m_pMesh->GetPBRMaterial().IsMetal = true;
            m_pMesh->GetPBRMaterial().Color = m_meshData[i].Color;
            m_phongRenderer.Render(m_pMesh);
        }

        float Roughness[] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };

        for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_meshData) ; i++) {
            m_pMesh->SetPosition(m_meshData[i].Pos + Vector3f(0.0f, 15.0f, 0.0f));
            m_pMesh->GetPBRMaterial().Roughness = Roughness[i];
            m_pMesh->GetPBRMaterial().IsMetal = false;
            m_pMesh->GetPBRMaterial().Color = Vector3f(0.1f, 0.33f, 0.17f);
            m_phongRenderer.Render(m_pMesh);
        }
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
        Vector3f Pos(0.0f, 35.0f, 0.0f);
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
        m_pMesh->LoadMesh("../Content/dragon.obj");
        m_pMesh->SetRotation(0.0f, 90.0f, 0.0f);
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

    struct {
        Vector3f Pos;
        Vector3f Color;
    } m_meshData[5];
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
