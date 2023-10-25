/*

        Copyright 2023 Etay Meiri

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

    Tutorial 45 - Point sprites with the Geometry Shader
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
#include "ogldev_billboard_list.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class Tutorial45
{
public:

    Tutorial45()
    {    
        m_dirLight.AmbientIntensity = 0.5f;
        m_dirLight.DiffuseIntensity = 0.9f;
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.WorldDirection = Vector3f(0.0f, -0.5f, 1.0f);

        m_position = Vector3f(0.0f, 0.0f, -12.0f);
    }


    virtual ~Tutorial45()
    {
        SAFE_DELETE(m_pGameCamera);
    }


    void Init()
    {
        CreateWindow();

        InitCallbacks();

        InitCamera();

        InitMesh();

        InitShaders();

        InitBillboardList();
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

        static float foo = 0.0f;
        foo += 0.001f;

        float TerrainSize = 20.0f;
        float Radius = TerrainSize * 1.4f;

        Vector3f Pos(cosf(foo) * Radius, 3.0f, sinf(foo) * Radius);
      //  m_pGameCamera->SetPosition(Pos);

        Vector3f Center(0, 0.5f, 0);
        Vector3f Target = Center - Pos;
    //    m_pGameCamera->SetTarget(Target);
    //    m_pGameCamera->SetUp(0.0f, 1.0f, 0.0f);


        m_pGameCamera->OnRender();

        ///////////////////////////
        // Render the main object
        ////////////////////////////

        Matrix4f CameraView = m_pGameCamera->GetMatrix();
        Matrix4f CameraProjection = m_pGameCamera->GetProjectionMat();
        Matrix4f VP = CameraProjection * CameraView;        
        m_billboardList.Render(VP, m_pGameCamera->GetPos());

        /////////////////////////
        // Render the terrain
        ////////////////////////

        m_lightingTech.Enable();

        // Set the WVP matrix from the camera point of view
        Matrix4f World = m_pTerrain->GetWorldMatrix();
        Matrix4f WVP = CameraProjection * CameraView * World;
        m_lightingTech.SetWVP(WVP);

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

            case GLFW_KEY_Z:
                m_isWireframe = !m_isWireframe;

                if (m_isWireframe) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
                else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 45");

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
        m_cameraPos = Vector3f(0.0f, 1.0f, -1.0f);
        m_cameraTarget = Vector3f(0.0f, -0.5f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);
	
        float FOV = 75.0f;
        float zNear = 0.1f;
        float zFar = 100.0f;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

        m_pGameCamera = new BasicCamera(persProjInfo, m_cameraPos, m_cameraTarget, Up);
        m_pGameCamera->SetSpeed(0.01f);
    }


    void InitShaders()
    {
        if (!m_lightingTech.Init()) {
            printf("Error initializing the lighting technique\n");
            exit(1);
        }

        m_lightingTech.Enable();
        m_lightingTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    }


    void InitMesh()
    {
        m_pTerrain = new BasicMesh();

        if (!m_pTerrain->LoadMesh("terrain.obj")) {
            printf("Error loading mesh terrain.obj\n");
            exit(0);
        }

        m_pTerrain->SetPosition(0.0f, 0.0f, 0.0f);
    }


    void InitBillboardList()
    {
#define NUM_ROWS 20
#define NUM_COLUMNS 20

        std::vector<Vector3f> Positions;
        Positions.resize(NUM_ROWS * NUM_COLUMNS);
        Vector3f Base(-20.0f, 0.0f, -20.0f);

        for (unsigned int j = 0; j < NUM_ROWS; j++) {
            for (unsigned int i = 0; i < NUM_COLUMNS; i++) {
                Vector3f Pos((float)i * 2.0f, 0.0f, (float)j * 2.0f);
                Positions[j * NUM_COLUMNS + i] = Base + Pos;
            }
        }

        //stylizedpaintleves01c
        //death-159120_1280
        if (!m_billboardList.Init("../Content/textures/death-159120_1280.png", Positions)) {
            printf("error\n");
            exit(0);
        }
    }

    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    LightingTechnique m_lightingTech;
    BasicMesh* m_pTerrain = NULL;
    DirectionalLight m_dirLight;
    Vector3f m_cameraPos;
    Vector3f m_cameraTarget;
    bool m_cameraOnLight = false;
    Vector3f m_position;
    BillboardList m_billboardList;
	bool m_isPaused = false;
    bool m_isWireframe = false;
};

Tutorial45* app = NULL;

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
    app = new Tutorial45();

    app->Init();

    glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0);

    app->Run();

    delete app;

    return 0;
}
