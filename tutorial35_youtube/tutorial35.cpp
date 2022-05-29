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

    Basic Shadow Mapping
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
#include "ogldev_shadow_map_fbo.h"
#include "ogldev_new_lighting.h"
#include "ogldev_shadow_mapping_technique.h"


#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

#define SHADOW_MAP_WIDTH 2048
#define SHADOW_MAP_HEIGHT 2048

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class Tutorial35
{
public:

    Tutorial35()
    {
        m_spotLight.WorldPosition  = Vector3f(-20.0, 20.0, 0.0f);
        m_spotLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_spotLight.DiffuseIntensity = 0.9f;
        m_spotLight.AmbientIntensity = 0.2f;
        m_spotLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_spotLight.Attenuation.Linear = 0.00f;
        m_spotLight.Attenuation.Exp = 0.0f;
        m_spotLight.Cutoff = 30.0f;

        // Initialize a perspective projection matrix for the spot light
        float FOV = 45.0f;
        float zNear = 1.0f;
        float zFar = 50.0f;
        PersProjInfo shadowPersProjInfo = { FOV, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, zNear, zFar };
        m_lightPersProjMatrix.InitPersProjTransform(shadowPersProjInfo);
    }

    virtual ~Tutorial35()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pMesh1);
    }


    void Init()
    {
        CreateWindow();

        CreateShadowMap();

        InitCallbacks();

        InitCamera();

        InitMesh();

        InitShaders();
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
        ShadowMapPass();
        LightingPass();
    }


    void ShadowMapPass()
    {
        m_shadowMapFBO.BindForWriting();

        glClear(GL_DEPTH_BUFFER_BIT);

        m_shadowMapTech.Enable();

        Matrix4f World = m_pMesh1->GetWorldMatrix();

        Matrix4f LightView;
        Vector3f Up(0.0f, 1.0f, 0.0f);
        LightView.InitCameraTransform(m_spotLight.WorldPosition, m_spotLight.WorldDirection, Up);

        Matrix4f WVP = m_lightPersProjMatrix * LightView * World;
        m_shadowMapTech.SetWVP(WVP);

        m_pMesh1->Render();
    }

    void LightingPass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_lightingTech.Enable();

        m_shadowMapFBO.BindForReading(SHADOW_TEXTURE_UNIT);

        m_pGameCamera->OnRender();

        static float foo = 0.0f;
        foo += 0.002f;

        m_spotLight.WorldPosition = Vector3f(-sinf(foo) * 15.0f, 8.0f, -cosf(foo) * 15.0f);
        m_spotLight.WorldDirection = m_pMesh1->GetPosition() - m_spotLight.WorldPosition;

        if (m_cameraOnLight) {
            m_pGameCamera->SetPosition(m_spotLight.WorldPosition);
            m_pGameCamera->SetTarget(m_spotLight.WorldDirection);
        }

        ///////////////////////////
        // Render the main object
        ////////////////////////////

        // Set the WVP matrix from the camera point of view
        Matrix4f World = m_pMesh1->GetWorldMatrix();
        Matrix4f CameraView = m_pGameCamera->GetMatrix();
        Matrix4f CameraProjection = m_pGameCamera->GetProjectionMat();
        Matrix4f WVP = CameraProjection * CameraView * World;
        m_lightingTech.SetWVP(WVP);

        // Set the WVP matrix from the light point of view
        Matrix4f LightView;
        Vector3f Up(0.0f, 1.0f, 0.0f);
        LightView.InitCameraTransform(m_spotLight.WorldPosition, m_spotLight.WorldDirection, Up);
        Matrix4f LightWVP = m_lightPersProjMatrix * LightView * World;
        m_lightingTech.SetLightWVP(LightWVP);

        Vector3f CameraLocalPos3f = m_pMesh1->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
        m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);

        m_spotLight.CalcLocalDirectionAndPosition(m_pMesh1->GetWorldTransform());

        m_lightingTech.SetSpotLights(1, &m_spotLight);

        m_lightingTech.SetMaterial(m_pMesh1->GetMaterial());

        m_pMesh1->Render();

        /////////////////////////
        // Render the terrain
        ////////////////////////

        // Set the WVP matrix from the camera point of view
        World = m_pTerrain->GetWorldMatrix();
        WVP = CameraProjection * CameraView * World;
        m_lightingTech.SetWVP(WVP);

        // Set the WVP matrix from the light point of view
        LightWVP = m_lightPersProjMatrix * LightView * World;
        m_lightingTech.SetLightWVP(LightWVP);

        // Update the shader with the local space pos/dir of the spot light
        m_spotLight.CalcLocalDirectionAndPosition(m_pTerrain->GetWorldTransform());
        m_lightingTech.SetSpotLights(1, &m_spotLight);
        m_lightingTech.SetMaterial(m_pTerrain->GetMaterial());

        // Update the shader with the local space pos of the camera
        CameraLocalPos3f = m_pTerrain->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
        m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);

        m_pTerrain->Render();
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

            case GLFW_KEY_L:
                m_cameraOnLight = !m_cameraOnLight;
                if (!m_cameraOnLight) {
                    m_pGameCamera->SetPosition(m_cameraPos);
                    m_pGameCamera->SetTarget(m_cameraTarget);
                }
                break;

            case GLFW_KEY_A:
                m_spotLight.Attenuation.Linear += ATTEN_STEP;
                m_spotLight.Attenuation.Linear += ATTEN_STEP;
                break;

            case GLFW_KEY_Z:
                m_spotLight.Attenuation.Linear -= ATTEN_STEP;
                m_spotLight.Attenuation.Linear -= ATTEN_STEP;
                break;

            case GLFW_KEY_S:
                m_spotLight.Attenuation.Exp += ATTEN_STEP;
                m_spotLight.Attenuation.Exp += ATTEN_STEP;
                break;

            case GLFW_KEY_X:
                m_spotLight.Attenuation.Exp -= ATTEN_STEP;
                m_spotLight.Attenuation.Exp -= ATTEN_STEP;
                break;
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 35");

        glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    }


    void CreateShadowMap()
    {
        if (!m_shadowMapFBO.Init(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT)) {
            exit(1);
        }
    }


    void InitCallbacks()
    {
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
    }


    void InitCamera()
    {
        m_cameraPos = Vector3f(3.0f, 3.0f, -15.0f);
        m_cameraTarget = Vector3f(-0.2f, -0.2f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        float FOV = 45.0f;
        float zNear = 1.0f;
        float zFar = 100.0f;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

        m_pGameCamera = new BasicCamera(persProjInfo, m_cameraPos, m_cameraTarget, Up);
    }


    void InitShaders()
    {
        if (!m_lightingTech.Init()) {
            printf("Error initializing the lighting technique\n");
            exit(1);
        }

        m_lightingTech.Enable();
        m_lightingTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_lightingTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);
        //    m_lightingTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

        if (!m_shadowMapTech.Init()) {
            printf("Error initializing the shadow mapping technique\n");
            exit(1);
        }
    }


    void InitMesh()
    {
        m_pMesh1 = new BasicMesh();

        m_pMesh1->LoadMesh("../Content/ordinary_house/ordinary_house.obj");

        //        m_pMesh1->LoadMesh("../Content/box.obj");
        //        m_pMesh1->SetPosition(0.0f, 1.0f, 0.0f);

        //m_pMesh1->LoadMesh("../Content/Vanguard.dae");
        //        m_pMesh1->SetPosition(0.0f, 3.5f, 0.0f);
        //        m_pMesh1->SetRotation(270.0f, 180.0f, 0.0f);

        m_pTerrain = new BasicMesh();
        m_pTerrain->LoadMesh("../Content/box_terrain.obj");
        m_pTerrain->SetPosition(0.0f, 0.0f, 0.0f);
    }

    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    LightingTechnique m_lightingTech;
    ShadowMappingTechnique m_shadowMapTech;
    BasicMesh* m_pMesh1 = NULL;
    BasicMesh* m_pTerrain = NULL;
    PersProjInfo m_persProjInfo;
    Matrix4f m_lightPersProjMatrix;
    SpotLight m_spotLight;
    ShadowMapFBO m_shadowMapFBO;
    Vector3f m_cameraPos;
    Vector3f m_cameraTarget;
    bool m_cameraOnLight = false;
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
