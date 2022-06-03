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

    Phong Reflection Model demo
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

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class PhongDemo
{
public:

    PhongDemo()
    {
        pointLights[0].WorldPosition = Vector3f(10.0f, 0.0f, 0.0f);
        pointLights[0].DiffuseIntensity = 1.0f;
        pointLights[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
        pointLights[0].Attenuation.Linear = 0.1f;
        pointLights[0].Attenuation.Exp = 0.0f;

        pointLights[1].WorldPosition = Vector3f(10.0f, 0.0f, 0.0f);
        pointLights[1].DiffuseIntensity = 0.25f;
        pointLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
        pointLights[1].Attenuation.Linear = 0.0f;
        pointLights[1].Attenuation.Exp = 0.2f;

        spotLights[0].DiffuseIntensity = 1.0f;
        spotLights[0].Color = Vector3f(1.0f, 0.0f, 0.0f);
        spotLights[0].Attenuation.Linear = 0.1f;
        spotLights[0].Cutoff = 5.0f;
        spotLights[0].WorldDirection = Vector3f(0.0f, 0.0f, 1.0f);

        spotLights[1].DiffuseIntensity = 0.0f;
        spotLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
        spotLights[1].Attenuation.Linear = 0.01f;
        spotLights[1].Cutoff = 30.0f;
        spotLights[1].WorldPosition = Vector3f(0.0f, 1.0f, 0.0f);
        spotLights[1].WorldDirection = Vector3f(0.0f, -1.0f, 0.0f);
    }

    virtual ~PhongDemo()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(pMesh1);
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

#ifdef _WIN64
        float YRotationAngle = 0.1f;
#else
        float YRotationAngle = 1.0f;
#endif
        counter += 0.01f;

        pointLights[1].WorldPosition.y = sinf(counter) * 4 + 4;
        m_phongRenderer.UpdatePointLightPos(1, pointLights[1].WorldPosition);

        spotLights[0].WorldPosition = m_pGameCamera->GetPos();
        spotLights[0].WorldDirection = m_pGameCamera->GetTarget();
        m_phongRenderer.UpdateSpotLightPosAndDir(0, spotLights[0].WorldPosition, spotLights[0].WorldDirection);

        long long CurrentTimeMillis = GetCurrentTimeMillis();
        float AnimationTimeSec = ((float)(CurrentTimeMillis - m_startTimeMillis)) / 1000.0f;

        m_phongRenderer.RenderAnimation(pMesh1, AnimationTimeSec);
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
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(0);

        case 'a':
            pointLights[0].Attenuation.Linear += ATTEN_STEP;
            pointLights[1].Attenuation.Linear += ATTEN_STEP;
            break;

        case 'z':
            pointLights[0].Attenuation.Linear -= ATTEN_STEP;
            pointLights[1].Attenuation.Linear -= ATTEN_STEP;
            break;

        case 's':
            pointLights[0].Attenuation.Exp += ATTEN_STEP;
            pointLights[1].Attenuation.Exp += ATTEN_STEP;
            break;

        case 'x':
            pointLights[0].Attenuation.Exp -= ATTEN_STEP;
            pointLights[1].Attenuation.Exp -= ATTEN_STEP;
            break;

        case 'd':
            spotLights[0].Cutoff += ANGLE_STEP;
            break;

        case 'c':
            spotLights[0].Cutoff -= ANGLE_STEP;
            break;

        case 'g':
            spotLights[1].Cutoff += ANGLE_STEP;
            break;

        case 'b':
            spotLights[1].Cutoff -= ANGLE_STEP;
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Phong Reflection Model Demo");

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
        m_phongRenderer.InitPhongRenderer();
        m_phongRenderer.SetCamera(m_pGameCamera);
        m_phongRenderer.SetPointLights(2, pointLights);
        m_phongRenderer.SetSpotLights(2, spotLights);
    }


    void InitMesh()
    {
        //        pMesh1 = new BasicMesh();
        //        pMesh1->LoadMesh("../Content/ordinary_house/ordinary_house.obj");
        // pMesh1->SetPosition(0.0f, 0.0f, 10.0f);

        pMesh1 = new SkinnedMesh();
        pMesh1->LoadMesh("../Content/Vanguard2.fbx");
        pMesh1->SetPosition(0.0f, 0.0f, 15.0f);
        //        pMesh1->SetRotation(270.0f, 180.0f, 0.0f);
    }

    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    PhongRenderer m_phongRenderer;
    SkinnedMesh* pMesh1 = NULL;
    PersProjInfo persProjInfo;
    PointLight pointLights[LightingTechnique::MAX_POINT_LIGHTS];
    SpotLight spotLights[LightingTechnique::MAX_SPOT_LIGHTS];
    float counter = 0;
    long long m_startTimeMillis = 0;
};

PhongDemo* app = NULL;

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
    app = new PhongDemo();

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
