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

    Improving Shadow Mapping
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

#define SHADOW_MAP_WIDTH 4096
#define SHADOW_MAP_HEIGHT 4096

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class Tutorial36
{
public:

    Tutorial36()
    {
        m_dirLight.AmbientIntensity = 0.5f;
        m_dirLight.DiffuseIntensity = 0.9f;
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.WorldDirection = Vector3f(1.0f, -0.1f, 0.0f);

        // Initialize an orthographic projection matrix for the directional light
        OrthoProjInfo shadowOrthoProjInfo;
        shadowOrthoProjInfo.l = -20.0f;
        shadowOrthoProjInfo.r = 20.0f;
        shadowOrthoProjInfo.t = 20.0f;
        shadowOrthoProjInfo.b = -20.0f;
        shadowOrthoProjInfo.n = -20.0f;
        shadowOrthoProjInfo.f = 20.0f;

        m_lightOrthoProjMatrix.InitOrthoProjTransform(shadowOrthoProjInfo);

        OrthoProjInfo cameraOrthoProjInfo;
        cameraOrthoProjInfo.l = -WINDOW_WIDTH / 250.0f;
        cameraOrthoProjInfo.r = WINDOW_WIDTH / 250.0f;
        cameraOrthoProjInfo.t = WINDOW_HEIGHT / 250.0f;
        cameraOrthoProjInfo.b = -WINDOW_HEIGHT / 250.0f;
        cameraOrthoProjInfo.n = 1.0f;
        cameraOrthoProjInfo.f = 100.0f;

        m_cameraOrthoProjMatrix.InitOrthoProjTransform(cameraOrthoProjInfo);

        m_positions[0] = Vector3f(0.0f, 0.0f, -12.0f);
        //  m_positions[1] = Vector3f(0.0f, 0.0f, 0.0f);
        //  m_positions[2] = Vector3f(0.0f, 0.0f, 15.0f);
    }


    virtual ~Tutorial36()
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
        static float foo = 0.001f;
        foo += 0.005f;
        if (foo >= 1.0f) {
            foo = 0.001f;
        }

        m_dirLight.WorldDirection = Vector3f(foo, -1.0f + foo, 0.0f);

        ShadowMapPass();
        LightingPass();
    }


    void ShadowMapPass()
    {
        m_shadowMapFBO.BindForWriting();

        glClear(GL_DEPTH_BUFFER_BIT);

        m_shadowMapTech.Enable();

        glCullFace(GL_FRONT);

        Vector3f LightPosWorld;
        OrthoProjInfo LightOrthoProjMatrix;
        CalcTightLightProjection(m_pGameCamera->GetMatrix(),
                                 m_dirLight.WorldDirection,
                                 m_pGameCamera->m_persProjInfo,
                                 LightPosWorld,
                                 LightOrthoProjMatrix);

        Matrix4f LightView;
        Vector3f Origin(0.0f, 0.0f, 0.0f);
        Vector3f Up(0.0f, 1.0f, 0.0f);

        for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_positions) ; i++) {
            m_pMesh1->SetPosition(m_positions[i]);
            Matrix4f World = m_pMesh1->GetWorldMatrix();
            LightView.InitCameraTransform(Origin, m_dirLight.WorldDirection, Up);
            Matrix4f WVP = m_lightOrthoProjMatrix * LightView * World;
            m_shadowMapTech.SetWVP(WVP);
            m_pMesh1->Render();
        }
    }

    void LightingPass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_lightingTech.Enable();

        glCullFace(GL_BACK);

        m_shadowMapFBO.BindForReading(SHADOW_TEXTURE_UNIT);

        m_pGameCamera->OnRender();

        if (m_cameraOnLight) {
            m_pGameCamera->SetPosition(m_dirLight.WorldDirection * 2.0f + Vector3f(0.0f, 5.0f, 0.0f));
            m_pGameCamera->SetTarget(Vector3f(0.0f, 0.0f, 0.0f) - m_dirLight.WorldDirection);
        }

        ///////////////////////////
        // Render the main object
        ////////////////////////////

        Matrix4f CameraView = m_pGameCamera->GetMatrix();
        Matrix4f CameraProjection;

        if (m_isOrthoCamera) {
            CameraProjection = m_cameraOrthoProjMatrix;
        } else {
            CameraProjection = m_pGameCamera->GetProjectionMat();
        }

        Matrix4f LightView;
        Vector3f Origin(0.0f, 0.0f, 0.0f);
        Vector3f Up(0.0f, 1.0f, 0.0f);
        LightView.InitCameraTransform(Origin, m_dirLight.WorldDirection, Up);

        m_lightingTech.SetMaterial(m_pMesh1->GetMaterial());

        for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_positions) ; i++) {
            // Set the WVP matrix from the camera point of view
            m_pMesh1->SetPosition(m_positions[i]);
            Matrix4f World = m_pMesh1->GetWorldMatrix();
            Matrix4f WVP = CameraProjection * CameraView * World;
            m_lightingTech.SetWVP(WVP);

            // Set the WVP matrix from the light point of view
            Matrix4f LightWVP = m_lightOrthoProjMatrix * LightView * World;
            m_lightingTech.SetLightWVP(LightWVP);

            Vector3f CameraLocalPos3f = m_pMesh1->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
            m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);
            m_dirLight.CalcLocalDirection(m_pMesh1->GetWorldTransform());
            m_lightingTech.SetDirectionalLight(m_dirLight);
            m_pMesh1->Render();
        }

        /////////////////////////
        // Render the terrain
        ////////////////////////

        // Set the WVP matrix from the camera point of view
        Matrix4f World = m_pTerrain->GetWorldMatrix();
        Matrix4f WVP = CameraProjection * CameraView * World;
        m_lightingTech.SetWVP(WVP);

        // Set the WVP matrix from the light point of view
        Matrix4f LightWVP = m_lightOrthoProjMatrix * LightView * World;
        m_lightingTech.SetLightWVP(LightWVP);

        // Update the shader with the local space pos/dir of the spot light
        m_dirLight.CalcLocalDirection(m_pTerrain->GetWorldTransform());
        m_lightingTech.SetDirectionalLight(m_dirLight);
        m_lightingTech.SetMaterial(m_pTerrain->GetMaterial());

        // Update the shader with the local space pos of the camera
        Vector3f CameraLocalPos3f = m_pTerrain->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
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
                m_dirLight.AmbientIntensity += ATTEN_STEP;
                break;

            case GLFW_KEY_D:
                m_dirLight.DiffuseIntensity -= ATTEN_STEP;
                break;

            case GLFW_KEY_P:
                m_isOrthoCamera = !m_isOrthoCamera;
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 36");

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
        m_cameraPos = Vector3f(3.0f, 3.0f, -25.0f);
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

        //m_pMesh1->LoadMesh("../Content/cylinder.obj");
                if (!m_pMesh1->LoadMesh("../Content/low_poly_rpg_collection/rpg_items_3.obj")) {
            printf("Error loading mesh ../Content/low_poly_rpg_collection/rpg_items_3.obj\n");
                    exit(0);
                }
        //        m_pMesh1->LoadMesh("../Content/ordinary_house/ordinary_house.obj");
        //m_pMesh1->LoadMesh("../Content/simple-afps-level.obj");

        //        m_pMesh1->LoadMesh("../Content/box.obj");
        //        m_pMesh1->SetPosition(0.0f, 1.0f, 0.0f);

        //m_pMesh1->LoadMesh("../Content/Vanguard.dae");
        //        m_pMesh1->SetPosition(0.0f, 3.5f, 0.0f);
        //        m_pMesh1->SetRotation(270.0f, 180.0f, 0.0f);

        m_pTerrain = new BasicMesh();
        if (!m_pTerrain->LoadMesh("../Content/box_terrain.obj")) {
            printf("Error loading mesh ../Content/box_terrain.obj\n");
            exit(0);
        }
        m_pTerrain->SetPosition(0.0f, 0.0f, 0.0f);
    }

    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    LightingTechnique m_lightingTech;
    ShadowMappingTechnique m_shadowMapTech;
    BasicMesh* m_pMesh1 = NULL;
    BasicMesh* m_pTerrain = NULL;
    Matrix4f m_lightOrthoProjMatrix;
    Matrix4f m_cameraOrthoProjMatrix;
    DirectionalLight m_dirLight;
    ShadowMapFBO m_shadowMapFBO;
    Vector3f m_cameraPos;
    Vector3f m_cameraTarget;
    bool m_cameraOnLight = false;
    Vector3f m_positions[1];
    bool m_isOrthoCamera = false;
};

Tutorial36* app = NULL;

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
    app = new Tutorial36();

    app->Init();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    app->Run();

    delete app;

    return 0;
}
