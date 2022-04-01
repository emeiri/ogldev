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

    Tutorial 31 - 3D Picking
*/

#include <math.h>
#include <GL/glew.h>


#include "ogldev_engine_common.h"
#include "ogldev_util.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_new_lighting.h"
#include "ogldev_glfw.h"
#include "ogldev_basic_mesh.h"
#include "picking_texture.h"
#include "picking_technique.h"
#include "simple_color_technique.h"
#include "ogldev_world_transform.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class Tutorial31
{
public:

    Tutorial31()
    {
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 3.0f;
        m_directionalLight.DiffuseIntensity = 0.1f;
        m_directionalLight.WorldDirection = Vector3f(-1.0f, 0.0, 0.0);

        // The same mesh will be rendered at the following locations
        m_worldPos[0] = Vector3f(-10.0f, 0.0f, 5.0f);
        m_worldPos[1] = Vector3f(10.0f, 0.0f, 5.0f);
        m_worldPos[2] = Vector3f(0.0f, 2.0f, 20.0f);
    }

    virtual ~Tutorial31()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(pMesh);
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
        m_pGameCamera->OnRender();

        // There's no point in the picking phase when the mouse is not pressed
        if (m_leftMouseButton.IsPressed) {
            PickingPhase();
        }

        RenderPhase();
    }


    void PickingPhase()
    {
        m_pickingTexture.EnableWriting();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pickingEffect.Enable();

        WorldTrans& worldTransform = pMesh->GetWorldTransform();
        Matrix4f View = m_pGameCamera->GetMatrix();
        Matrix4f Projection = m_pGameCamera->GetProjectionMat();

        for (uint i = 0 ; i < (int)ARRAY_SIZE_IN_ELEMENTS(m_worldPos) ; i++) {
            worldTransform.SetPosition(m_worldPos[i]);
            // Background is zero, the real objects start at 1
            m_pickingEffect.SetObjectIndex(i + 1);
            Matrix4f World = worldTransform.GetMatrix();
            Matrix4f WVP = Projection * View * World;
            m_pickingEffect.SetWVP(WVP);
            pMesh->Render(&m_pickingEffect);
        }

        m_pickingTexture.DisableWriting();
    }


    void RenderPhase()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        WorldTrans& worldTransform = pMesh->GetWorldTransform();
        Matrix4f View = m_pGameCamera->GetMatrix();
        Matrix4f Projection = m_pGameCamera->GetProjectionMat();

        // If the left mouse button is clicked check if it hit a triangle
        // and color it red
        int clicked_object_id = -1;
        if (m_leftMouseButton.IsPressed) {
            PickingTexture::PixelInfo Pixel = m_pickingTexture.ReadPixel(m_leftMouseButton.x, WINDOW_HEIGHT - m_leftMouseButton.y - 1);

            if (Pixel.ObjectID != 0) {
                // Compensate for the SetObjectindex call in the picking phase
                clicked_object_id = Pixel.ObjectID - 1;
                assert(clicked_object_id < ARRAY_SIZE_IN_ELEMENTS(m_worldPos));
                m_simpleColorEffect.Enable();
                worldTransform.SetPosition(m_worldPos[clicked_object_id]);
                Matrix4f World = worldTransform.GetMatrix();
                Matrix4f WVP = Projection * View * World;
                m_simpleColorEffect.SetWVP(WVP);
                pMesh->Render(Pixel.DrawID, Pixel.PrimID);
            }
        }

        // render the objects as usual
        m_lightingEffect.Enable();

        for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_worldPos) ; i++) {
            worldTransform.SetPosition(m_worldPos[i]);
            Matrix4f World = worldTransform.GetMatrix();
            Matrix4f WVP = Projection * View * World;
            m_lightingEffect.SetWVP(WVP);
            Vector3f CameraLocalPos3f = worldTransform.WorldPosToLocalPos(m_pGameCamera->GetPos());
            m_lightingEffect.SetCameraLocalPos(CameraLocalPos3f);
            m_directionalLight.CalcLocalDirection(worldTransform);
            m_lightingEffect.SetDirectionalLight(m_directionalLight);

            if (i == clicked_object_id) {
                 m_lightingEffect.SetColorMod(Vector4f(0.0f, 1.0, 0.0, 1.0f));
            } else {
                 m_lightingEffect.SetColorMod(Vector4f(1.0f, 1.0, 1.0, 1.0f));
            }

            pMesh->Render(NULL);
        }
    }


     void KeyboardCB(uint key, int state)
    {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(0);
            break;

        case 'a':
            m_directionalLight.AmbientIntensity += 0.05f;
            break;

        case 's':
            m_directionalLight.AmbientIntensity -= 0.05f;
            break;

        case 'z':
            m_directionalLight.DiffuseIntensity += 0.05f;
            break;

        case 'x':
            m_directionalLight.DiffuseIntensity -= 0.05f;
            break;
        default:
            m_pGameCamera->OnKeyboard(key);
        }
    }


    void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }


    void MouseCB(int button, int action, int x, int y)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            m_leftMouseButton.IsPressed = (action == GLFW_PRESS);
            m_leftMouseButton.x = x;
            m_leftMouseButton.y = y;
        }
    }

private:

    void CreateWindow()
    {
        int major_ver = 0;
        int minor_ver = 0;
        bool is_full_screen = false;
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 31");
    }


    void InitCallbacks()
    {
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
    }


    void InitCamera()
    {
        Vector3f Pos(0.0f, 5.0f, -22.0f);
        Vector3f Target(0.0f, -0.2f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        float FOV = 45.0f;
        float zNear = 0.1f;
        float zFar = 100.0f;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

        m_pGameCamera = new BasicCamera(persProjInfo, Pos, Target, Up);

        if (!m_lightingEffect.Init()) {
            printf("Error initializing the lighting technique\n");
            exit(1);
        }
    }


    void InitShaders()
    {
        m_lightingEffect.Enable();
        m_lightingEffect.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_lightingEffect.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);
        m_lightingEffect.SetMaterial(pMesh->GetMaterial());

        m_pickingTexture.Init(WINDOW_WIDTH, WINDOW_HEIGHT);

        if (!m_pickingEffect.Init()) {
            exit(1);
        }

        if (!m_simpleColorEffect.Init()) {
            exit(1);
        }
    }


    void InitMesh()
    {
        pMesh = new BasicMesh();

        pMesh->LoadMesh("../Content/spider.obj");

        WorldTrans& worldTransform = pMesh->GetWorldTransform();
        worldTransform.SetScale(0.1f);
        worldTransform.SetRotation(0.0f, 90.0f, 0.0f);
    }

    GLFWwindow* window = NULL;
    LightingTechnique m_lightingEffect;
    PickingTechnique m_pickingEffect;
    SimpleColorTechnique m_simpleColorEffect;
    BasicCamera* m_pGameCamera = NULL;
    DirectionalLight m_directionalLight;
    BasicMesh* pMesh = NULL;
    PickingTexture m_pickingTexture;
    struct {
        bool IsPressed = false;
        int x;
        int y;
    } m_leftMouseButton;
    Vector3f m_worldPos[3];
};


Tutorial31* app = NULL;

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
    app = new Tutorial31();

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
