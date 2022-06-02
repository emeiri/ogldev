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

    Tutorial 23 - Spot Light
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

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440



class Tutorial23
{
public:
    Tutorial23();
    ~Tutorial23();

    bool Init();

    void RenderSceneCB();
    void KeyboardCB(unsigned char key, int mouse_x, int mouse_y);
    void SpecialKeyboardCB(int key, int mouse_x, int mouse_y);
    void PassiveMouseCB(int x, int y);

private:

    GLuint WVPLocation;
    GLuint SamplerLocation;
    Camera* pGameCamera = NULL;
    BasicMesh* pBox = NULL;
    BasicMesh* pMesh1 = NULL;
    BasicMesh* pMesh2 = NULL;
    PersProjInfo persProjInfo;
    LightingTechnique* pLightingTech = NULL;
    PointLight pointLights[LightingTechnique::MAX_POINT_LIGHTS];
    SpotLight spotLights[LightingTechnique::MAX_SPOT_LIGHTS];
    float counter = 0;
};


Tutorial23::Tutorial23()
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

    pointLights[0].DiffuseIntensity = 0.5f;
    pointLights[0].Color = Vector3f(1.0f, 1.0f, 0.0f);
    pointLights[0].Attenuation.Linear = 0.2f;
    pointLights[0].Attenuation.Exp = 0.0f;

    pointLights[1].DiffuseIntensity = 0.0f;
    pointLights[1].Color = Vector3f(0.0f, 1.0f, 1.0f);
    pointLights[1].Attenuation.Linear = 0.0f;
    pointLights[1].Attenuation.Exp = 0.2f;

    spotLights[0].DiffuseIntensity = 1.0f;
    spotLights[0].Color = Vector3f(1.0f, 0.0f, 0.0f);
    spotLights[0].Attenuation.Linear = 0.01f;
    spotLights[0].Cutoff = 20.0f;

    spotLights[1].DiffuseIntensity = 1.0f;
    spotLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
    spotLights[1].Attenuation.Linear = 0.01f;
    spotLights[1].Cutoff = 30.0f;
}


Tutorial23::~Tutorial23()
{
    if (pGameCamera) {
        delete pGameCamera;
    }

    if (pBox) {
        delete pBox;
    }

    if (pMesh1) {
        delete pMesh1;
    }

    if (pMesh2) {
        delete pMesh2;
    }

    if (pLightingTech) {
        delete pLightingTech;
    }
}


bool Tutorial23::Init()
{
    Vector3f CameraPos(0.0f, 5.0f, -8.0f);
    Vector3f CameraTarget(0.0f, -0.5f, 1.0f);
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, CameraPos, CameraTarget, CameraUp);

    pBox = new BasicMesh();

    if (!pBox->LoadMesh("../Content/box_terrain.obj")) {
        return false;
    }

    pMesh1 = new BasicMesh();

    if (!pMesh1->LoadMesh("../Content/Zombie.obj")) {
    //    if (!pMesh1->LoadMesh("/home/emeiri/Downloads/vintage_wooden_drawer_01_4k.blend/vintage_wooden_drawer_01_4k.obj")) {
        return false;
    }

    pMesh2 = new BasicMesh();

    if (!pMesh2->LoadMesh("/home/emeiri/diskb/emeiri/Downloads/vintage_grandfather_clock_01_4k.blend/vintage_grandfather_clock_01_4k.obj")) {
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

bool Start = false;

void Tutorial23::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGameCamera->OnRender();

#ifdef _WIN64
    float YRotationAngle = 0.1f;
#else
    float YRotationAngle = 1.0f;
#endif
    counter += 0.01f;

    WorldTrans& worldTransform = pBox->GetWorldTransform();

    worldTransform.SetRotation(0.0f, 0.0f, 0.0f);
    worldTransform.SetPosition(0.0f, 0.0f, 10.0f);

    Matrix4f World = worldTransform.GetMatrix();
    Matrix4f View = pGameCamera->GetMatrix();
    Matrix4f Projection;
    Projection.InitPersProjTransform(persProjInfo);
    Matrix4f WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pointLights[0].WorldPosition.x = -10.0f;
    pointLights[0].WorldPosition.y = 2;
    pointLights[0].WorldPosition.z = 0.0f;
    pointLights[0].CalcLocalPosition(worldTransform);

    pointLights[1].WorldPosition.x = 10.0f;
    pointLights[1].WorldPosition.y = sinf(counter) * 4 + 4;
    pointLights[1].WorldPosition.z = 0.0f;
    pointLights[1].CalcLocalPosition(worldTransform);

    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].WorldPosition = pGameCamera->GetPos();
    spotLights[0].WorldDirection = pGameCamera->GetTarget();
    spotLights[0].CalcLocalDirectionAndPosition(worldTransform);

    spotLights[1].WorldPosition = Vector3f(0.0f, 1.0f, 0.0f);
    spotLights[1].WorldDirection = Vector3f(0.0f, -1.0f, 0.0f);
    spotLights[1].CalcLocalDirectionAndPosition(worldTransform);

    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(pBox->GetMaterial());

    Vector3f CameraLocalPos3f = worldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    pBox->Render();

    WorldTrans& meshWorldTransform = pMesh1->GetWorldTransform();
    if (Start) {
        static float counter2 = 0.0f;
        counter2 += 0.001;
        meshWorldTransform.SetPosition(0.0f, -4.0f + abs(sinf(counter2) * 4), 0.0f);
    } else {
        meshWorldTransform.SetPosition(0.0f, -4.0f, 0.0f);
    }

    World = meshWorldTransform.GetMatrix();
    WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pointLights[0].CalcLocalPosition(meshWorldTransform);
    pointLights[1].CalcLocalPosition(meshWorldTransform);
    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].CalcLocalDirectionAndPosition(meshWorldTransform);
    spotLights[1].CalcLocalDirectionAndPosition(meshWorldTransform);
    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(pMesh1->GetMaterial());

    CameraLocalPos3f = meshWorldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    pMesh1->Render();

    WorldTrans& meshWorldTransform2 = pMesh2->GetWorldTransform();
    //  meshWorldTransform2.SetRotation(0.0f, -45.0f, 0.0f);
    meshWorldTransform2.SetPosition(0.0f, 1.0f, 1.0f);
    World = meshWorldTransform2.GetMatrix();
    WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pointLights[0].CalcLocalPosition(meshWorldTransform2);
    pointLights[1].CalcLocalPosition(meshWorldTransform2);
    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].CalcLocalDirectionAndPosition(meshWorldTransform2);
    spotLights[1].CalcLocalDirectionAndPosition(meshWorldTransform2);
    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(pMesh2->GetMaterial());

    CameraLocalPos3f = meshWorldTransform2.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    pMesh2->Render();

    glutPostRedisplay();
    glutSwapBuffers();
}


#define ATTEN_STEP 0.01f

#define ANGLE_STEP 1.0f

void Tutorial23::KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    switch (key) {
    case ' ':
        Start = true;
        break;
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

    case 'd':
        spotLights[0].Cutoff += ANGLE_STEP;
        break;

    case 'c':
        spotLights[0].Cutoff -= ANGLE_STEP;
        break;

    case 'g':
        spotLights[1].Cutoff += ANGLE_STEP;
        break;

    case 'b':
        spotLights[1].Cutoff -= ANGLE_STEP;
        break;

    }

    printf("Linear %f Exp %f\n", pointLights[0].Attenuation.Linear, pointLights[0].Attenuation.Exp);

    pGameCamera->OnKeyboard(key);
}


void Tutorial23::SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGameCamera->OnKeyboard(key);
}


void Tutorial23::PassiveMouseCB(int x, int y)
{
    pGameCamera->OnMouse(x, y);
}


Tutorial23* pTutorial23 = NULL;


void RenderSceneCB()
{
    pTutorial23->RenderSceneCB();
}


void KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    pTutorial23->KeyboardCB(key, mouse_x, mouse_y);
}


void SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pTutorial23->SpecialKeyboardCB(key, mouse_x, mouse_y);
}


void PassiveMouseCB(int x, int y)
{
    pTutorial23->PassiveMouseCB(x, y);
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
    int win = glutCreateWindow("Tutorial 23");
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

    pTutorial23 = new Tutorial23();

    if (!pTutorial23->Init()) {
        return 1;
    }

    glutMainLoop();

    return 0;
}
