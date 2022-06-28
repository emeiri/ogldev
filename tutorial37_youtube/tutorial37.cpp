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

    Basic Shadow Mapping With Point Lights
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <cfloat>

#include "ogldev_engine_common.h"
#include "ogldev_util.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_new_lighting.h"
#include "ogldev_glfw.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_world_transform.h"
#include "ogldev_shadow_cube_map_fbo.h"
#include "ogldev_new_lighting.h"
#include "ogldev_shadow_mapping_technique_point_light.h"


#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

#define SHADOW_MAP_SIZE 4096

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);

struct CameraDirection
{
    GLenum CubemapFace;
    Vector3f Target;
    Vector3f Up;
};

CameraDirection gCameraDirections[NUM_CUBE_MAP_FACES] =
{
    { GL_TEXTURE_CUBE_MAP_POSITIVE_X, Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f) },
    { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 0.0f, -1.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f) },
    { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 1.0f, 0.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 1.0f, 0.0f) }
};



class Tutorial37
{
public:

    Tutorial37()
    {
        m_pointLight.AmbientIntensity = 0.1f;
        m_pointLight.DiffuseIntensity = 0.9f;
        m_pointLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLight.WorldPosition = Vector3f(0.0f, 2.0f, 0.0f);

        float FOV = 90.0f;
        float zNear = 0.1f;
        float zFar = 20.0f;
        PersProjInfo shadowPersProjInfo = { FOV, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, zNear, zFar };
        m_lightPersProjMatrix.InitPersProjTransform(shadowPersProjInfo);

        OrthoProjInfo cameraOrthoProjInfo;
        cameraOrthoProjInfo.l = -WINDOW_WIDTH / 250.0f;
        cameraOrthoProjInfo.r = WINDOW_WIDTH / 250.0f;
        cameraOrthoProjInfo.t = WINDOW_HEIGHT / 250.0f;
        cameraOrthoProjInfo.b = -WINDOW_HEIGHT / 250.0f;
        cameraOrthoProjInfo.n = 1.0f;
        cameraOrthoProjInfo.f = 100.0f;

        m_cameraOrthoProjMatrix.InitOrthoProjTransform(cameraOrthoProjInfo);

        m_housePositions[0] = Vector3f(0.0f, 0.0f, -8.0f);    // near
        m_cylinderPositions[0] = Vector3f(0.0f, 0.0f, -4.0f);
        m_housePositions[1] = Vector3f(-8.0f, 0.0f, 0.0f);    // left
        m_cylinderPositions[1] = Vector3f(-4.0f, 0.0f, 1.0f);
        m_housePositions[2] = Vector3f(8.0f, 0.0f, 0.0f);     // right
        m_cylinderPositions[2] = Vector3f(4.0f, 0.0f, 1.0f);
        m_housePositions[3] = Vector3f(0.0f, 0.0f, 8.0f);     // far
        m_cylinderPositions[3] = Vector3f(0.0f, 0.0f, 4.0f);
    }


    virtual ~Tutorial37()
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
        m_shadowMapTech.Enable();

        m_shadowMapTech.SetLightWorldPos(m_pointLight.WorldPosition);

        glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);

        for (uint i = 0 ; i < NUM_CUBE_MAP_FACES ; i++) {

            m_shadowCubeMapFBO.BindForWriting(gCameraDirections[i].CubemapFace);

            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            Matrix4f LightView;
            LightView.InitCameraTransform(m_pointLight.WorldPosition, gCameraDirections[i].Target, gCameraDirections[i].Up);

            for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_housePositions) ; i++) {
                m_pMesh1->SetPosition(m_housePositions[i]);
                Matrix4f World = m_pMesh1->GetWorldMatrix();
                Matrix4f WVP = m_lightPersProjMatrix * LightView * World;
                m_shadowMapTech.SetWVP(WVP);
                m_shadowMapTech.SetWorld(World);
                m_pMesh1->Render();
            }

            for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_cylinderPositions) ; i++) {
                m_pMesh2->SetPosition(m_cylinderPositions[i]);
                Matrix4f World = m_pMesh2->GetWorldMatrix();
                Matrix4f WVP = m_lightPersProjMatrix * LightView * World;
                m_shadowMapTech.SetWVP(WVP);
                m_shadowMapTech.SetWorld(World);
                m_pMesh2->Render();
            }
        }
    }

    void LightingPass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_lightingTech.Enable();

        m_shadowCubeMapFBO.BindForReading(SHADOW_CUBE_MAP_TEXTURE_UNIT);

        m_pGameCamera->OnRender();

        static float foo = 0.0f;
        foo += 0.01f;

        if (m_cameraOnLight) {
            m_pGameCamera->SetPosition(m_pointLight.WorldPosition + Vector3f(0.0f, 2.0f, 0.0f));
            m_pGameCamera->SetTarget(Vector3f(sinf(foo), -0.35f, cosf(foo)));
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

        for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_housePositions) ; i++) {
            // Set the WVP matrix from the camera point of view
            m_pMesh1->SetPosition(m_housePositions[i]);
            Matrix4f World = m_pMesh1->GetWorldMatrix();
            Matrix4f WVP = CameraProjection * CameraView * World;
            m_lightingTech.SetWorldMatrix(World);
            m_lightingTech.SetWVP(WVP);

            Vector3f CameraLocalPos3f = m_pMesh1->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
            m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);
            m_pointLight.CalcLocalPosition(m_pMesh1->GetWorldTransform());
            m_lightingTech.SetPointLights(1, &m_pointLight);
            m_pMesh1->Render(&m_lightingTech);
        }

        for (int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_cylinderPositions) ; i++) {
            // Set the WVP matrix from the camera point of view
            m_pMesh2->SetPosition(m_cylinderPositions[i]);
            Matrix4f World = m_pMesh2->GetWorldMatrix();
            Matrix4f WVP = CameraProjection * CameraView * World;
            m_lightingTech.SetWorldMatrix(World);
            m_lightingTech.SetWVP(WVP);

            Vector3f CameraLocalPos3f = m_pMesh2->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
            m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);
            m_pointLight.CalcLocalPosition(m_pMesh2->GetWorldTransform());
            m_lightingTech.SetPointLights(1, &m_pointLight);
            m_pMesh2->Render(&m_lightingTech);
        }

        /////////////////////////
        // Render the terrain
        ////////////////////////
        // Set the WVP matrix from the camera point of view
        Matrix4f World = m_pTerrain->GetWorldMatrix();
        Matrix4f WVP = CameraProjection * CameraView * World;
        m_lightingTech.SetWorldMatrix(World);
        m_lightingTech.SetWVP(WVP);

        // Update the shader with the local space pos/dir of the spot light
        m_pointLight.CalcLocalPosition(m_pTerrain->GetWorldTransform());
        m_lightingTech.SetPointLights(1, &m_pointLight);

        // Update the shader with the local space pos of the camera
        Vector3f CameraLocalPos3f = m_pTerrain->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
        m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);

        m_pTerrain->Render(&m_lightingTech);
    }


#define ATTEN_STEP 0.01f

#define ANGLE_STEP 1.0f

    void PassiveMouseCB(int x, int y)
    {
        if (!m_cameraOnLight) {
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

            case GLFW_KEY_L:
                m_cameraOnLight = !m_cameraOnLight;
                if (!m_cameraOnLight) {
                    m_pGameCamera->SetPosition(m_cameraPos);
                    m_pGameCamera->SetTarget(m_cameraTarget);
                }
                break;

            case GLFW_KEY_A:
                m_pointLight.AmbientIntensity += ATTEN_STEP;
                break;

            case GLFW_KEY_D:
                m_pointLight.DiffuseIntensity -= ATTEN_STEP;
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 37");

        glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    }


    void CreateShadowMap()
    {
        if (!m_shadowCubeMapFBO.Init(SHADOW_MAP_SIZE)) {
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
        m_cameraPos = Vector3f(0.0f, 15.0f, -15.0f);
        m_cameraTarget = Vector3f(0.0f, -0.5f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        float FOV = 90.0f;
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
        m_lightingTech.SetShadowCubeMapTextureUnit(SHADOW_CUBE_MAP_TEXTURE_UNIT_INDEX);
        //    m_lightingTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

        if (!m_shadowMapTech.Init()) {
            printf("Error initializing the shadow mapping technique\n");
            exit(1);
        }
    }


    void InitMesh()
    {
        m_pMesh1 = new BasicMesh();

        m_pMesh1->LoadMesh("../Content/low_poly_rpg_collection/rpg_items_3.obj");

        m_pMesh2 = new BasicMesh();

        m_pMesh2->LoadMesh("../Content/cylinder.obj");

        //        m_pMesh1->LoadMesh("../Content/ordinary_house/ordinary_house.obj");
        //m_pMesh1->LoadMesh("../Content/simple-afps-level.obj");

        //m_pMesh1->LoadMesh("../Content/box.obj");
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
    ShadowMappingPointLightTechnique m_shadowMapTech;
    BasicMesh* m_pMesh1 = NULL;
    BasicMesh* m_pMesh2 = NULL;
    BasicMesh* m_pTerrain = NULL;
    Matrix4f m_lightPersProjMatrix;
    Matrix4f m_cameraOrthoProjMatrix;
    PointLight m_pointLight;
    ShadowCubeMapFBO m_shadowCubeMapFBO;
    Vector3f m_cameraPos;
    Vector3f m_cameraTarget;
    bool m_cameraOnLight = false;
    Vector3f m_housePositions[4] = {};
    Vector3f m_cylinderPositions[4] = {};
    bool m_isOrthoCamera = false;
};

Tutorial37* app = NULL;

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
    app = new Tutorial37();

    app->Init();

    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    app->Run();

    delete app;

    return 0;
}
