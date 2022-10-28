/*

        Copyright 2021 Etay Meiri

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

    Forward Renderer Demo
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>


#include "ogldev_engine_common.h"
#include "ogldev_util.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_glfw.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_world_transform.h"
#include "ogldev_forward_renderer.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class ForwardRendererDemo
{
public:

    ForwardRendererDemo()
    {
        m_pointLights[0].WorldPosition = Vector3f(10.0f, 0.0f, 0.0f);
        m_pointLights[0].DiffuseIntensity = 1.0f;
        m_pointLights[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLights[0].Attenuation.Linear = 0.1f;
        m_pointLights[0].Attenuation.Exp = 0.0f;

        m_pointLights[1].WorldPosition = Vector3f(10.0f, 0.0f, 0.0f);
        m_pointLights[1].DiffuseIntensity = 0.25f;
        m_pointLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLights[1].Attenuation.Linear = 0.0f;
        m_pointLights[1].Attenuation.Exp = 0.2f;

        m_spotLights[0].DiffuseIntensity = 1.0f;
        m_spotLights[0].Color = Vector3f(1.0f, 0.0f, 0.0f);
        m_spotLights[0].Attenuation.Linear = 0.1f;
        m_spotLights[0].Cutoff = 5.0f;
        m_spotLights[0].WorldDirection = Vector3f(0.0f, 0.0f, 1.0f);

        m_spotLights[1].DiffuseIntensity = 0.0f;
        m_spotLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_spotLights[1].Attenuation.Linear = 0.01f;
        m_spotLights[1].Cutoff = 30.0f;
        m_spotLights[1].WorldPosition = Vector3f(0.0f, 1.0f, 0.0f);
        m_spotLights[1].WorldDirection = Vector3f(0.0f, -1.0f, 0.0f);

        m_dirLight.WorldDirection = Vector3f(1.0f, 0.0f, 1.0f);
        m_dirLight.DiffuseIntensity = 1.0f;
    }

    virtual ~ForwardRendererDemo()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pMesh1);
    }


    void Init()
    {
        CreateWindow();

        InitCallbacks();

        InitCamera();

        InitMesh();

        InitRenderer();

        m_startTimeMillis = GetCurrentTimeMillis();
    }


    void Run()
    {
        while (!glfwWindowShouldClose(m_window)) {
            RenderSceneCB();
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }
    }


    void RenderSceneCB()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pGameCamera->OnRender();

#ifdef _WIN64
        float YRotationAngle = 0.1f;
#else
        float YRotationAngle = 1.0f;
#endif
        m_counter += 0.01f;

        /*        m_pointLights[1].WorldPosition.y = sinf(m_counter) * 4 + 4;
        m_renderer.UpdatePointLightPos(1, m_pointLights[1].WorldPosition);

        m_spotLights[0].WorldPosition = m_pGameCamera->GetPos();
        m_spotLights[0].WorldDirection = m_pGameCamera->GetTarget();
        m_renderer.UpdateSpotLightPosAndDir(0, m_spotLights[0].WorldPosition, m_spotLights[0].WorldDirection);
        */
        long long CurrentTimeMillis = GetCurrentTimeMillis();
        float AnimationTimeSec = ((float)(CurrentTimeMillis - m_startTimeMillis)) / 1000.0f;

        m_renderer.Render(m_pMesh1);
    }


#define ATTEN_STEP 0.01f

#define ANGLE_STEP 1.0f

    void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }

    void KeyboardCB(uint key, int state)
    {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwDestroyWindow(m_window);
            glfwTerminate();
            exit(0);

        case 'a':
            m_pointLights[0].Attenuation.Linear += ATTEN_STEP;
            m_pointLights[1].Attenuation.Linear += ATTEN_STEP;
            break;

        case 'z':
            m_pointLights[0].Attenuation.Linear -= ATTEN_STEP;
            m_pointLights[1].Attenuation.Linear -= ATTEN_STEP;
            break;

        case 's':
            m_pointLights[0].Attenuation.Exp += ATTEN_STEP;
            m_pointLights[1].Attenuation.Exp += ATTEN_STEP;
            break;

        case 'x':
            m_pointLights[0].Attenuation.Exp -= ATTEN_STEP;
            m_pointLights[1].Attenuation.Exp -= ATTEN_STEP;
            break;

        case 'd':
            m_spotLights[0].Cutoff += ANGLE_STEP;
            break;

        case 'c':
            m_spotLights[0].Cutoff -= ANGLE_STEP;
            break;

        case 'g':
            m_spotLights[1].Cutoff += ANGLE_STEP;
            break;

        case 'b':
            m_spotLights[1].Cutoff -= ANGLE_STEP;
            break;

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
        m_window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "ForwardRenderer Reflection Model Demo");

        glfwSetCursorPos(m_window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    }


    void InitCallbacks()
    {
        glfwSetKeyCallback(m_window, KeyCallback);
        glfwSetCursorPosCallback(m_window, CursorPosCallback);
        glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    }


    void InitCamera()
    {
        Vector3f Pos(0.0f, 0.0f, 0.0f);
        Vector3f Target(0.0f, 0.0f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        float FOV = 45.0f;
        float zNear = 0.1f;
        float zFar = 100.0f;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

        m_pGameCamera = new BasicCamera(persProjInfo, Pos, Target, Up);
    }


    void InitRenderer()
    {
        m_renderer.InitForwardRenderer();
        m_renderer.SetCamera(m_pGameCamera);
        //        m_renderer.SetPointLights(2, m_pointLights);
        //        m_renderer.SetSpotLights(2, m_spotLights);
        m_renderer.SetDirLight(m_dirLight);
    }


    void InitMesh()
    {
        //        pMesh1 = new BasicMesh();
        //        pMesh1->LoadMesh("../Content/ordinary_house/ordinary_house.obj");
        // pMesh1->SetPosition(0.0f, 0.0f, 10.0f);

        m_pMesh1 = new SkinnedMesh();
        m_pMesh1->LoadMesh("../Content/sphere.obj");
        m_pMesh1->SetPosition(0.0f, 0.0f, 15.0f);
        //        pMesh1->SetRotation(270.0f, 180.0f, 0.0f);
    }

    GLFWwindow* m_window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    ForwardRenderer m_renderer;
    SkinnedMesh* m_pMesh1 = NULL;
    PersProjInfo m_persProjInfo;
    PointLight m_pointLights[ForwardLightingTechnique::MAX_POINT_LIGHTS];
    SpotLight m_spotLights[ForwardLightingTechnique::MAX_SPOT_LIGHTS];
    DirectionalLight m_dirLight;
    float m_counter = 0;
    long long m_startTimeMillis = 0;
};

ForwardRendererDemo* app = NULL;

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
    app = new ForwardRendererDemo();

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
