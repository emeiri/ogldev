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

    Toon Shading & Rim Lighting
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


class Tutorial35
{
public:

    Tutorial35()
    {
        m_pointLights[0].WorldPosition = Vector3f(20.0f, 0.0f, 0.0f);
        m_pointLights[0].DiffuseIntensity = 1.0f;
        m_pointLights[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLights[0].Attenuation.Linear = 0.1f;
        m_pointLights[0].Attenuation.Exp = 0.0f;

        m_pointLights[1].WorldPosition = Vector3f(10.0f, 0.0f, 0.0f);
        m_pointLights[1].DiffuseIntensity = 0.25f;
        m_pointLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLights[1].Attenuation.Linear = 0.0f;
        m_pointLights[1].Attenuation.Exp = 0.2f;

        m_dirLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_dirLight.DiffuseIntensity = 1.0f;
        m_dirLight.AmbientIntensity = 0.1f;
    }

    virtual ~Tutorial35()
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
        foo += 0.002f;

        m_pGameCamera->SetPosition(-sinf(foo) * 13.0f, 8.0f, -cosf(foo) * 13.0f);
        Vector3f Target(m_pMesh1->GetPosition() - m_pGameCamera->GetPos() + Vector3f(0.0f, 3.0f, 0.0f));
        m_pGameCamera->SetTarget(Target);

        m_dirLight.WorldDirection = Vector3f(sinf(foo), -0.5f, cosf(foo));
        m_phongRenderer.UpdateDirLightDir(m_dirLight.WorldDirection);
        m_phongRenderer.ControlRimLight(m_isRimLightEnabled);
        m_phongRenderer.ControlCellShading(m_isCellShadingEnabled);
        m_phongRenderer.Render(m_pMesh1);

        // Don't use rim lighting and cell shading for the terrain
        m_phongRenderer.ControlRimLight(false);
        m_phongRenderer.ControlCellShading(false);
        m_phongRenderer.Render(m_pTerrain);
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

            case 'a':
                m_pointLights[0].Attenuation.Linear += ATTEN_STEP;
                m_pointLights[1].Attenuation.Linear += ATTEN_STEP;
                break;

            case GLFW_KEY_C:
                m_isCellShadingEnabled = !m_isCellShadingEnabled;
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

            case GLFW_KEY_R:
                m_isRimLightEnabled = !m_isRimLightEnabled;
                break;
            }
        }

        //        printf("Linear %f Exp %f\n", m_pointLights[0].Attenuation.Linear, m_pointLights[0].Attenuation.Exp);

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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 35");

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
        //m_phongRenderer.SetPointLights(1, m_pointLights);
        m_phongRenderer.SetDirLight(m_dirLight);
    }


    void InitMesh()
    {
        m_pMesh1 = new BasicMesh();
        //m_pMesh1->LoadMesh("../Content/ordinary_house/ordinary_house.obj");

        m_pMesh1->LoadMesh("../Content/Vanguard.dae");
        m_pMesh1->SetPosition(0.0f, 0.0f, 10.0f);
        m_pMesh1->SetRotation(270.0f, 180.0f, 0.0f);

        m_pTerrain = new BasicMesh();
        m_pTerrain->LoadMesh("../Content/box_terrain.obj");
        m_pTerrain->SetPosition(0.0f, 0.0f, 0.0f);
    }

    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    PhongRenderer m_phongRenderer;
    BasicMesh* m_pMesh1 = NULL;
    BasicMesh* m_pTerrain = NULL;
    PersProjInfo m_persProjInfo;
    PointLight m_pointLights[LightingTechnique::MAX_POINT_LIGHTS];
    bool m_isRimLightEnabled = false;
    bool m_isCellShadingEnabled = false;
    DirectionalLight m_dirLight;
};

Tutorial35* app = NULL;

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
    app = new Tutorial35();

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
