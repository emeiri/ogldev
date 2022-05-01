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

    Tutorial 33 - Sprite Batching
*/

#include <math.h>
#include <GL/glew.h>

#include "ogldev_engine_common.h"
#include "ogldev_util.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_glfw.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_tex_technique.h"
#include "sprite_batch.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);


class Tutorial33
{
public:

    Tutorial33()
    {
    }

    virtual ~Tutorial33()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pMesh);
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
        Sprites.resize(5);

        Sprites[0].PixelX = anim_pixelX;
        Sprites[0].PixelY = 0;
        Sprites[0].SpriteRow = anim_row;
        Sprites[0].SpriteCol = 5;
        Sprites[0].SpriteWidth = 1000;

        Sprites[1].PixelX = 500;
        Sprites[1].PixelY = 0;
        Sprites[1].SpriteRow = anim_row;
        Sprites[1].SpriteCol = 5;
        Sprites[1].SpriteWidth = 500;

        Sprites[2].PixelX = 750;
        Sprites[2].PixelY = 0;
        Sprites[2].SpriteRow = anim_row;
        Sprites[2].SpriteCol = 5;
        Sprites[2].SpriteWidth = 250;

        Sprites[3].PixelX = 875;
        Sprites[3].PixelY = 0;
        Sprites[3].SpriteRow = anim_row;
        Sprites[3].SpriteCol = 5;
        Sprites[3].SpriteWidth = 125;

        Sprites[4].PixelX = 875 + 73;
        Sprites[4].PixelY = 0;
        Sprites[4].SpriteRow = anim_row;
        Sprites[4].SpriteCol = 5;
        Sprites[4].SpriteWidth = 73;

        long long CurTime = GetCurrentTimeMillis();

        if ((CurTime - m_prevTime) > 100) {
            anim_row--;

            if (anim_row == 0) {
                anim_row = 7;
            }

            m_prevTime = CurTime;

            anim_pixelX += 4;

            if (anim_pixelX == 1000) {
                anim_pixelX = 0;
            }
        }

        m_pSpriteBatch->Render(Sprites);
        //m_pSpriteBatch->RenderAll();

        m_texTech.Enable();
        m_pTexture->Bind(COLOR_TEXTURE_UNIT);
        m_pMesh->Render();

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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 33");

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
        if (!m_texTech.Init()) {
            printf("Error initializing the texture technique\n");
            exit(1);
        }

        m_texTech.Enable();
        m_texTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    }


    void InitMesh()
    {
        m_pMesh = new BasicMesh();

        m_pMesh->LoadMesh("../Content/fs_quad.obj");

        m_pTexture = new Texture(GL_TEXTURE_2D, "../Content/craftpix.net.Cartoon_Forest_BG_01.png");
        m_pTexture->Load();
    }


    void InitSpriteBatch()
    {
        uint NumSpritesX = 6;
        uint NumSpritesY = 8;
        m_pSpriteBatch = new SpriteBatch("../Content/spritesheet.png", NumSpritesX, NumSpritesY, WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    GLFWwindow* window = NULL;
    TexTechnique m_texTech;
    BasicCamera* m_pGameCamera = NULL;
    bool m_mobileCamera = false;
    BasicMesh* m_pMesh = NULL;
    Texture* m_pTexture = NULL;
    SpriteBatch* m_pSpriteBatch = NULL;
    long long m_prevTime = 0;

    int anim_row = 7;
    int anim_col = 0;
    int anim_pixelX = 0;
};


Tutorial33* app = NULL;

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
    app = new Tutorial33();

    app->Init();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    GLint num;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &num);
    printf("%d\n", num);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &num);
    printf("%d\n", num);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &num);
    printf("%d\n", num);
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &num);
    printf("%d\n", num);
    //    exit(0);
    app->Run();

    delete app;

    return 0;
}
