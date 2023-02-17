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

    Soft Shadows With PCF & Random Sampling
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
#include "ogldev_shadow_map_offset_texture.h"


#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

#define SHADOW_MAP_WIDTH 4096
#define SHADOW_MAP_HEIGHT 4096

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class Tutorial41
{
public:

    Tutorial41()
    {
        m_dirLight.AmbientIntensity = 0.5f;
        m_dirLight.DiffuseIntensity = 0.9f;
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.WorldDirection = Vector3f(1.0f, -0.75f, 0.0f);

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
    }


    virtual ~Tutorial41()
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
        static float foo = 0.0f;
        //        foo += 0.01f;

        //        m_dirLight.WorldDirection = Vector3f(sinf(foo), -0.5f, cosf(foo));

        ShadowMapPass();
        LightingPass();
    }


    void ShadowMapPass()
    {
        m_shadowMapFBO.BindForWriting();

        glClear(GL_DEPTH_BUFFER_BIT);

        m_shadowMapTech.Enable();

        glCullFace(GL_FRONT); // Solution #2 from the video - reverse face culling

        Matrix4f LightView;
        Vector3f Origin(0.0f, 0.0f, 0.0f);
        Vector3f Up(0.0f, 1.0f, 0.0f);

        Matrix4f World = m_pMesh1->GetWorldMatrix();
        LightView.InitCameraTransform(Origin, m_dirLight.WorldDirection, Up);
        Matrix4f WVP = m_lightOrthoProjMatrix * LightView * World;
        m_shadowMapTech.SetWVP(WVP);
        m_pMesh1->Render();
    }

    void LightingPass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_lightingTech.Enable();

        glCullFace(GL_BACK); // Solution #2 from the video - reverse face culling

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

        // Set the WVP matrix from the camera point of view
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

        /////////////////////////
        // Render the terrain
        ////////////////////////

        // Set the WVP matrix from the camera point of view
        World = m_pTerrain->GetWorldMatrix();
        WVP = CameraProjection * CameraView * World;
        m_lightingTech.SetWVP(WVP);

        // Set the WVP matrix from the light point of view
        LightWVP = m_lightOrthoProjMatrix * LightView * World;
        m_lightingTech.SetLightWVP(LightWVP);

        // Update the shader with the local space pos/dir of the spot light
        m_dirLight.CalcLocalDirection(m_pTerrain->GetWorldTransform());
        m_lightingTech.SetDirectionalLight(m_dirLight);
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

            case GLFW_KEY_P:
                m_isOrthoCamera = !m_isOrthoCamera;
                break;

            case GLFW_KEY_A:
                m_shadowMapFilterSize++;
                printf("Shadow map filter size increased to %d\n", m_shadowMapFilterSize);
                m_lightingTech.SetShadowMapFilterSize(m_shadowMapFilterSize);
                break;

            case GLFW_KEY_Z:
                if (m_shadowMapFilterSize > 0) {
                    m_shadowMapFilterSize--;
                    printf("Shadow map filter size decreased to %d\n", m_shadowMapFilterSize);
                    m_lightingTech.SetShadowMapFilterSize(m_shadowMapFilterSize);
                }
                break;

            case GLFW_KEY_S:
                m_shadowMapSampleRadius++;
                printf("Shadow map radius increased to %f\n", m_shadowMapSampleRadius);
                m_lightingTech.SetShadowMapOffsetTextureParams((float)m_shadowMapOffsetTextureSize,
                                                               (float)m_shadowMapOffsetFilterSize,
                                                               m_shadowMapSampleRadius);
                break;

            case GLFW_KEY_X:
                if (m_shadowMapSampleRadius > 0) {
                    m_shadowMapSampleRadius--;
                    printf("Shadow map radius decrease to %f\n", m_shadowMapSampleRadius);
                    m_lightingTech.SetShadowMapOffsetTextureParams((float)m_shadowMapOffsetTextureSize,
                                                                   (float)m_shadowMapOffsetFilterSize,
                                                                   m_shadowMapSampleRadius);
                }
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 41");

        glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    }


    void CreateShadowMap()
    {
        bool ForPCF = true;

        if (!m_shadowMapFBO.Init(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, ForPCF)) {
            exit(1);
        }

        m_pShadowMapOffsetTexture = new ShadowMapOffsetTexture(m_shadowMapOffsetTextureSize,
                                                               m_shadowMapOffsetFilterSize);

        m_pShadowMapOffsetTexture->Bind(SHADOW_MAP_RANDOM_OFFSET_TEXTURE_UNIT);
    }


    void InitCallbacks()
    {
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
    }


    void InitCamera()
    {
        m_cameraPos = Vector3f(-24.0f, 10.0f, -31.0f);
        m_cameraTarget = Vector3f(0.5f, -0.2f, 1.0f);
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
        m_lightingTech.SetShadowMapSize(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
        m_lightingTech.SetShadowMapFilterSize(m_shadowMapFilterSize);
        m_lightingTech.SetShadowMapOffsetTextureUnit(SHADOW_MAP_RANDOM_OFFSET_TEXTURE_UNIT_INDEX);

        m_lightingTech.SetShadowMapOffsetTextureParams((float)m_shadowMapOffsetTextureSize,
                                                       (float)m_shadowMapOffsetFilterSize,
                                                       m_shadowMapSampleRadius);
        //    m_lightingTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

        if (!m_shadowMapTech.Init()) {
            printf("Error initializing the shadow mapping technique\n");
            exit(1);
        }
    }


    void InitMesh()
    {
        m_pMesh1 = new BasicMesh();
        m_pMesh1->LoadMesh("../Content/dragon.obj");
        m_pMesh1->SetPosition(-15.0f, 0.0f, 0.0f);
        m_pMesh1->SetRotation(0.0f, 90.0f, 0.0f);

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
    bool m_isOrthoCamera = false;
    ShadowMapOffsetTexture* m_pShadowMapOffsetTexture = NULL;
    int m_shadowMapFilterSize = 0;
    float m_shadowMapSampleRadius = 0.0f;
    int m_shadowMapOffsetTextureSize = 16;
    int m_shadowMapOffsetFilterSize = 8;
};

Tutorial41* app = NULL;

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
    app = new Tutorial41();

    app->Init();

    glClearColor(0.4f, 0.4f, 0.4f, 0.4f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    app->Run();

    delete app;

    return 0;
}
