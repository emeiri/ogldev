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

    Tutorial 44 - User Clip Planes
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
#include "ogldev_new_lighting.h"


#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class Tutorial44
{
public:

    Tutorial44()
    {
        m_dirLight.AmbientIntensity = 0.5f;
        m_dirLight.DiffuseIntensity = 0.9f;
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.WorldDirection = Vector3f(0.0f, -0.5f, 1.0f);

        m_position = Vector3f(0.0f, 0.0f, -12.0f);
    }


    virtual ~Tutorial44()
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
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_lightingTech.Enable();

        m_pGameCamera->OnRender();

        static float foo = 0.0f;
        foo += 0.002f;
        float R = 10.0f;

        Vector3f Pos(m_position.x + cosf(foo) * R, 7.0f, m_position.z + sinf(foo) * R);
      //  m_pGameCamera->SetPosition(Pos);
        Vector3f Target = m_position - Pos;
        Target.y += 1.0f;
    //    m_pGameCamera->SetTarget(Target);
     //   m_pGameCamera->SetUp(0.0f, 1.0f, 0.0f);

        ///////////////////////////
        // Render the main object
        ////////////////////////////

        Matrix4f CameraView = m_pGameCamera->GetMatrix();
        Matrix4f CameraProjection = m_pGameCamera->GetProjectionMat();

        m_lightingTech.SetMaterial(m_pMesh1->GetMaterial());

        Vector3f PlaneNormal(sinf(m_clipPlaneAngle), -1.0f, 0.0f);
        Vector3f PointOnPlane(0.0f, m_clipPlaneHeight, 0.0f);

        m_lightingTech.SetClipPlane(PlaneNormal, PointOnPlane);

        // Set the WVP matrix from the camera point of view
        m_pMesh1->SetPosition(m_position);
        Matrix4f World = m_pMesh1->GetWorldMatrix();
        Matrix4f WVP = CameraProjection * CameraView * World;        
        m_lightingTech.SetWVP(WVP);

        Vector3f CameraLocalPos3f = m_pMesh1->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
        m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);
        m_dirLight.CalcLocalDirection(m_pMesh1->GetWorldTransform());
        m_lightingTech.SetDirectionalLight(m_dirLight);
        m_pMesh1->Render();

        /////////////////////////
        // Render the terrain
        ////////////////////////

        PlaneNormal = Vector3f(0.0f, -1.0f, 0.0f);
        m_lightingTech.SetClipPlane(PlaneNormal, PointOnPlane);

        // Set the WVP matrix from the camera point of view
        World = m_pTerrain->GetWorldMatrix();
        WVP = CameraProjection * CameraView * World;
        m_lightingTech.SetWVP(WVP);

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
        if (!m_isPaused) {
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

            case GLFW_KEY_P:
                m_isPaused = !m_isPaused;
                break;

            case GLFW_KEY_A:
                m_clipPlaneHeight += 0.1f;
                break;

            case GLFW_KEY_Z:
                m_clipPlaneHeight -= 0.1f;
                break;

            case GLFW_KEY_X:
                m_clipPlaneAngle += 0.1f;
                break;

            case GLFW_KEY_C:
                m_clipPlaneAngle -= 0.1f;
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 44");

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
        m_cameraPos = Vector3f(3.0f, 5.0f, -31.0f);
        m_cameraTarget = Vector3f(0.0f, -0.1f, 1.0f);

        Vector3f Up(0.0, 1.0f, 0.0f);

        float FOV = 45.0f;
        float zNear = 0.1f;
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
    }


    void InitMesh()
    {
        m_pMesh1 = new BasicMesh();

        m_pMesh1->LoadMesh("../Content/ordinary_house/ordinary_house.obj");

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
    BasicMesh* m_pMesh1 = NULL;
    BasicMesh* m_pTerrain = NULL;
    Matrix4f m_cameraOrthoProjMatrix;
    Vector3f m_lightWorldPos;
    Matrix4f m_lightOrthoProjMatrix;
    DirectionalLight m_dirLight;
    Vector3f m_cameraPos;
    Vector3f m_cameraTarget;
    bool m_cameraOnLight = false;
    Vector3f m_position;
    float m_clipPlaneHeight = 1.0f;
    float m_clipPlaneAngle = 0.0f;
    bool m_isPaused = false;
};

Tutorial44* app = NULL;

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
    app = new Tutorial44();

    app->Init();

    glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
    glFrontFace(GL_CW);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0);

    app->Run();

    delete app;

    return 0;
}
