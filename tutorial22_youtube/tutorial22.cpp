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

    Tutorial 22 - Point Light
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
#include "camera.h"
#include "lighting_technique.h"
#include "ogldev_engine_common.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

// Workaround for tutorials prior to switching to GLFW
int IsGLVersionHigher(int MajorVer, int MinorVer)
{
    return false;
}


class Tutorial22
{
public:
    Tutorial22();
    ~Tutorial22();

    bool Init();

    void RenderSceneCB();
    void KeyboardCB(unsigned char key, int mouse_x, int mouse_y);
    void SpecialKeyboardCB(int key, int mouse_x, int mouse_y);
    void PassiveMouseCB(int x, int y);

private:

    GLuint WVPLocation;
    GLuint SamplerLocation;
    Camera* pGameCamera = NULL;
    BasicMesh* pMesh = NULL;
    PersProjInfo persProjInfo;
    LightingTechnique* pLightingTech = NULL;
    PointLight pointLights[LightingTechnique::MAX_POINT_LIGHTS];
    float counter = 0;
};


Tutorial22::Tutorial22()
{
    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;

    persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

    pointLights[0].DiffuseIntensity = 1.0f;
    pointLights[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
    pointLights[0].Attenuation.Linear = 0.2f;
    pointLights[0].Attenuation.Exp = 0.0f;

    pointLights[1].DiffuseIntensity = 1.0f;
    pointLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
    pointLights[1].Attenuation.Linear = 0.0f;
    pointLights[1].Attenuation.Exp = 0.2f;
}


Tutorial22::~Tutorial22()
{
    if (pGameCamera) {
        delete pGameCamera;
    }

    if (pMesh) {
        delete pMesh;
    }

    if (pLightingTech) {
        delete pLightingTech;
    }
}


bool Tutorial22::Init()
{
    Vector3f CameraPos(0.0f, 5.0f, -8.0f);
    Vector3f CameraTarget(0.0f, -0.5f, 1.0f);
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, CameraPos, CameraTarget, CameraUp);

    pMesh = new BasicMesh();

    if (!pMesh->LoadMesh("../Content/box_terrain.obj")) {
        return false;
    }

    pLightingTech = new LightingTechnique();

    if (!pLightingTech->Init())
    {
        return false;
    }

    pLightingTech->Enable();

    pLightingTech->SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    pLightingTech->SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    return true;
}


void Tutorial22::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGameCamera->OnRender();

#ifdef _WIN64
    float YRotationAngle = 0.1f;
#else
    float YRotationAngle = 1.0f;
#endif

    WorldTrans& worldTransform = pMesh->GetWorldTransform();

    worldTransform.SetRotation(0.0f, 0.0f, 0.0f);
    worldTransform.SetPosition(0.0f, 0.0f, 10.0f);

    Matrix4f World = worldTransform.GetMatrix();
    Matrix4f View = pGameCamera->GetMatrix();
    Matrix4f Projection;
    Projection.InitPersProjTransform(persProjInfo);
    Matrix4f WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    counter += 0.01f;
    pointLights[0].WorldPosition.x = -8.0f;
    pointLights[0].WorldPosition.y = sinf(counter) * 4 + 4;
    pointLights[0].WorldPosition.z = 0.0f;
    pointLights[0].CalcLocalPosition(worldTransform);

    pointLights[1].WorldPosition.x = 8.0f;
    pointLights[1].WorldPosition.y = sinf(counter) * 4 + 4;
    pointLights[1].WorldPosition.z = 0.0f;
    pointLights[1].CalcLocalPosition(worldTransform);

    pLightingTech->SetPointLights(2, pointLights);

    pLightingTech->SetMaterial(pMesh->GetMaterial());

    Vector3f CameraLocalPos3f = worldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    pMesh->Render();

    glutPostRedisplay();
    glutSwapBuffers();
}


#define ATTEN_STEP 0.01f

void Tutorial22::KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    switch (key) {
    case 'q':
    case 27:    // escape key code
        exit(0);

    case 'a':
        pointLights[0].Attenuation.Linear += ATTEN_STEP;
        pointLights[1].Attenuation.Linear += ATTEN_STEP;
        break;

    case 'z':
        pointLights[0].Attenuation.Linear -= ATTEN_STEP;
        pointLights[1].Attenuation.Linear -= ATTEN_STEP;
        break;

    case 's':
        pointLights[0].Attenuation.Exp += ATTEN_STEP;
        pointLights[1].Attenuation.Exp += ATTEN_STEP;
        break;

    case 'x':
        pointLights[0].Attenuation.Exp -= ATTEN_STEP;
        pointLights[1].Attenuation.Exp -= ATTEN_STEP;
        break;

    }

    printf("Linear %f Exp %f\n", pointLights[0].Attenuation.Linear, pointLights[0].Attenuation.Exp);

    pGameCamera->OnKeyboard(key);
}


void Tutorial22::SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGameCamera->OnKeyboard(key);
}


void Tutorial22::PassiveMouseCB(int x, int y)
{
    pGameCamera->OnMouse(x, y);
}


Tutorial22* pTutorial22 = NULL;


void RenderSceneCB()
{
    pTutorial22->RenderSceneCB();
}


void KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    pTutorial22->KeyboardCB(key, mouse_x, mouse_y);
}


void SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pTutorial22->SpecialKeyboardCB(key, mouse_x, mouse_y);
}


void PassiveMouseCB(int x, int y)
{
    pTutorial22->PassiveMouseCB(x, y);
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
    SRANDOM;

    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    int x = 200;
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("Tutorial 22");
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

    pTutorial22 = new Tutorial22();

    if (!pTutorial22->Init()) {
        return 1;
    }

    glutMainLoop();

    return 0;
}
