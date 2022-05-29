/*

        Copyright 2021 Etay Meiri

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

    OpenGL QNA - Order of transformations
*/

#include <stdio.h>
#include <string.h>

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_math_3d.h"
#include "ogldev_texture.h"
#include "ogldev_world_transform.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_engine_common.h"
#include "camera.h"
#include "simple_technique.h"
#include "lighting_technique.h"


#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440


static GLuint CreateVertexBuffer()
{
    Vector3f Vertices[6];
    Vertices[0] = Vector3f(-1000.0f, 0.0f, 0.0f);
    Vertices[1] = Vector3f(1000.0f, 0.0f, 0.0f);
    Vertices[2] = Vector3f(0.0f, -1000.0f, 0.0f);
    Vertices[3] = Vector3f(0.0f, 1000.0f, 0.0f);
    Vertices[4] = Vector3f(0.0f, 0.0f, -1000.0f);
    Vertices[5] = Vector3f(0.0f, 0.0f, 1000.0f);

    GLuint VAO = 0;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO = 0;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return VAO;
}



class TransformOrder
{
public:
    TransformOrder();
    ~TransformOrder();

    bool Init();

    void RenderSceneCB();
    void KeyboardCB(unsigned char key, int mouse_x, int mouse_y);
    void SpecialKeyboardCB(int key, int mouse_x, int mouse_y);
    void PassiveMouseCB(int x, int y);

private:

    void RenderCoordinateSystem();

    void RenderBox();

    GLuint WVPLocation;
    GLuint SamplerLocation;
    Camera* pGameCamera = NULL;
    BasicMesh* pMesh = NULL;
    PersProjInfo persProjInfo;
    Matrix4f Projection;
    SimpleTechnique* pSimpleTech = NULL;
    LightingTechnique* pLightingTech = NULL;
    BaseLight baseLight;
    GLuint CoordSystem;
};


TransformOrder::TransformOrder()
{
    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    float FOV = 45.0f;
    float zNear = 1.0f;
    float zFar = 100.0f;

    persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };
    Projection.InitPersProjTransform(persProjInfo);

    baseLight.AmbientIntensity = 1.0f;
}


TransformOrder::~TransformOrder()
{
    if (pGameCamera) {
        delete pGameCamera;
    }

    if (pMesh) {
        delete pMesh;
    }

    if (pSimpleTech) {
        delete pSimpleTech;
    }

    if (pLightingTech) {
        delete pLightingTech;
    }
}


bool TransformOrder::Init()
{
    Vector3f CameraPos(0.0f, 2.0f, -6.0f);
    Vector3f CameraTarget(0.0f, -0.4f, 1.0f);
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, CameraPos, CameraTarget, CameraUp);

    pMesh = new BasicMesh();

    if (!pMesh->LoadMesh("../../Content/box.obj")) {
        return false;
    }

    CoordSystem = CreateVertexBuffer();

    pLightingTech = new LightingTechnique();

    if (!pLightingTech->Init())
    {
        return false;
    }

    pSimpleTech = new SimpleTechnique();

    if (!pSimpleTech->Init())
    {
        return false;
    }

    pLightingTech->Enable();

    pLightingTech->SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

    return true;
}


void TransformOrder::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGameCamera->OnRender();

    RenderCoordinateSystem();

    RenderBox();

    glutPostRedisplay();

    glutSwapBuffers();
}


void TransformOrder::RenderCoordinateSystem()
{
    Matrix4f World;
    World.InitIdentity();

    Matrix4f View = pGameCamera->GetMatrix();

    Matrix4f WVP = Projection * View * World;

    pSimpleTech->Enable();
    pSimpleTech->SetWVP(WVP);

    glBindVertexArray(CoordSystem);
    glDrawArrays(GL_LINES, 0, 6);
}


void TransformOrder::RenderBox()
{
    static float RotationAngle = 0.0f;

#ifdef _WIN64
    float RotationStep = 0.1f;
#else
    float RotationStep = 1.0f;
#endif

    RotationAngle += RotationStep;

    Matrix4f RotationMatrix;
    RotationMatrix.InitRotateTransform(RotationAngle, 0.0f, 0.0f);

    Matrix4f TranslationMatrix;
    TranslationMatrix.InitTranslationTransform(4.0f, 0.0f, 0.0f);

    //Matrix4f World = RotationMatrix;
    //Matrix4f World = RotationMatrix * TranslationMatrix;
    Matrix4f World = TranslationMatrix * RotationMatrix;

    Matrix4f View = pGameCamera->GetMatrix();

    Matrix4f WVP = Projection * View * World;

    pLightingTech->Enable();
    pLightingTech->SetWVP(WVP);
    pLightingTech->SetLight(baseLight);
    pLightingTech->SetMaterial(pMesh->GetMaterial());

    pMesh->Render();

    RotationMatrix.InitRotateTransform(0.0f, RotationAngle, 0.0f);
    TranslationMatrix.InitTranslationTransform(-1.0f, 0.0f, 0.0f);
    World =  RotationMatrix * TranslationMatrix;
    WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pMesh->Render();

    RotationMatrix.InitRotateTransform(0.0f, RotationAngle, 0.0f);
    TranslationMatrix.InitTranslationTransform(0.0f, 3.0f, 0.0f);
    World =  RotationMatrix * TranslationMatrix;
    WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pMesh->Render();
}

void TransformOrder::KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    switch (key) {
    case 'q':
    case 27:    // escape key code
        exit(0);
    }

    pGameCamera->OnKeyboard(key);
}


void TransformOrder::SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGameCamera->OnKeyboard(key);
}


void TransformOrder::PassiveMouseCB(int x, int y)
{
    pGameCamera->OnMouse(x, y);
}


TransformOrder* pTransformOrder = NULL;


void RenderSceneCB()
{
    pTransformOrder->RenderSceneCB();
}


void KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    pTransformOrder->KeyboardCB(key, mouse_x, mouse_y);
}


void SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pTransformOrder->SpecialKeyboardCB(key, mouse_x, mouse_y);
}


void PassiveMouseCB(int x, int y)
{
    pTransformOrder->PassiveMouseCB(x, y);
}


void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
    glutKeyboardFunc(KeyboardCB);
    glutSpecialFunc(SpecialKeyboardCB);
    glutPassiveMotionFunc(PassiveMouseCB);
}

int main(int argc, char** argv)
{
#ifdef _WIN64
    srand(GetCurrentProcessId());
#else
    srandom(getpid());
#endif

    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    int x = 200;
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("Order Of Transformations");
    printf("window id: %d\n", win);

    // char game_mode_string[64];
    // Game mode string example: <Width>x<Height>@<FPS>
    // Enable the following three lines for full screen
    // snprintf(game_mode_string, sizeof(game_mode_string), "%dx%d@60", WINDOW_WIDTH, WINDOW_HEIGHT);
    // glutGameModeString(game_mode_string);
    // glutEnterGameMode();

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    InitializeGlutCallbacks();

    pTransformOrder = new TransformOrder();

    if (!pTransformOrder->Init()) {
        return 1;
    }

    glutMainLoop();

    return 0;
}
