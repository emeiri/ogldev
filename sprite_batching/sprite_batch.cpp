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

    Sprite Batching
*/

#include <math.h>
#include <GL/glew.h>


#include "ogldev_engine_common.h"
#include "ogldev_util.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_new_lighting.h"
#include "ogldev_glfw.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_world_transform.h"
#include "ogldev_sprite_batch.h"


#define WINDOW_WIDTH  2000
#define WINDOW_HEIGHT 1000

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class Tutorial32
{
public:

    Tutorial32()
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

    virtual ~Tutorial32()
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

        InitSpriteBatch();
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
        if (m_mobileCamera) {
            m_pGameCamera->OnRender();
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vector<SpriteBatch::SpriteInfo> Sprites;
        Sprites.resize(2);

        /*              uint PixelX = 0;
        uint PixelY = 0;
        uint SpriteRow = 0;
        uint SpriteCol = 0;
        uint SpriteWidth = 0;*/

        static int row = 7;
        static int col = 0;
        Sprites[0].PixelX = 1000;
        Sprites[0].PixelY = 0;
        Sprites[0].SpriteRow = row;
        Sprites[0].SpriteCol = 5;
        Sprites[0].SpriteWidth = 1000;

        Sprites[1].PixelX = 550;
        Sprites[1].PixelY = 0;
        Sprites[1].SpriteRow = row;
        Sprites[1].SpriteCol = 5;
        Sprites[1].SpriteWidth = 100;

        long long CurTime = GetCurrentTimeMillis();

        if ((CurTime - m_prevTime) > 100) {
            row--;
            if (row == 0) {
                row = 7;
            }
            m_prevTime = CurTime;
        }


        m_pSpriteBatch->Render(Sprites);
        //m_pSpriteBatch->RenderAll();


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

        case ' ':
            if (state == GLFW_PRESS) {
                m_mobileCamera = !m_mobileCamera;
            }
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
        if (m_mobileCamera) {
            m_pGameCamera->OnMouse(x, y);
        }
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 32");

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
        Vector3f Pos(0.0f, 5.0f, -22.0f);
        Vector3f Target(0.0f, -0.2f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        float FOV = 45.0f;
        float zNear = 0.1f;
        float zFar = 100.0f;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

        m_pGameCamera = new BasicCamera(persProjInfo, Pos, Target, Up);
    }


    void InitShaders()
    {
        if (!m_lightingEffect.Init()) {
            printf("Error initializing the lighting technique\n");
            exit(1);
        }

        m_lightingEffect.Enable();
        m_lightingEffect.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_lightingEffect.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);
        m_lightingEffect.SetMaterial(pMesh->GetMaterial());
    }


    void InitMesh()
    {
        pMesh = new BasicMesh();

        pMesh->LoadMesh("../Content/box.obj");

        WorldTrans& worldTransform = pMesh->GetWorldTransform();
        //        worldTransform.SetScale(0.1f);
        //        worldTransform.SetRotation(0.0f, 90.0f, 0.0f);
    }


    void InitSpriteBatch()
    {
        uint NumSpritesX = 6;
        uint NumSpritesY = 8;
        m_pSpriteBatch = new SpriteBatch("../Content/spritesheet.png", NumSpritesX, NumSpritesY, WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    GLFWwindow* window = NULL;
    LightingTechnique m_lightingEffect;
    BasicCamera* m_pGameCamera = NULL;
    bool m_mobileCamera = false;
    DirectionalLight m_directionalLight;
    BasicMesh* pMesh = NULL;
    Vector3f m_worldPos[3];
    SpriteBatch* m_pSpriteBatch = NULL;
    long long m_prevTime = 0;
};


Tutorial32* app = NULL;

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
    app = new Tutorial32();

    app->Init();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    //    GLint num;
    //    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE/*GL_MAX_VERTEX_UNIFORM_COMPONENTS*/, &num);
    //    printf("%d\n", num);
    //    exit(0);
    app->Run();

    delete app;

    return 0;
}
