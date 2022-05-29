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
#include "3rdparty/stb_image_write.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080



class TexturedCube
{
public:
    TexturedCube();
    ~TexturedCube();

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
    PointLight pointLight;
    float counter = 0;
    unsigned char buffer[WINDOW_WIDTH * WINDOW_HEIGHT * 3];
};


TexturedCube::TexturedCube()
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

    pointLight.AmbientIntensity = 1.0f;
    pointLight.DiffuseIntensity = 1.0f;
    pointLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
    pointLight.Attenuation.Linear = 0.02f;
    pointLight.Attenuation.Exp = 0.0f;
}


TexturedCube::~TexturedCube()
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


bool TexturedCube::Init()
{
    Vector3f CameraPos(0.0f, 0.0f, -5.0f);
    Vector3f CameraTarget(0.0f, 0.0f, 1.0f);
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, CameraPos, CameraTarget, CameraUp);

    pMesh = new BasicMesh();

    if (!pMesh->LoadMesh("../Content/box.obj")) {
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


void TexturedCube::RenderSceneCB()
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
    worldTransform.SetPosition(0.0f, 0.0f, 1.0f);

    Matrix4f World = worldTransform.GetMatrix();
    Matrix4f View = pGameCamera->GetMatrix();
    Matrix4f Projection;
    Projection.InitPersProjTransform(persProjInfo);
    Matrix4f WVP = Projection * View * World;

    pLightingTech->SetWVP(WVP);

    counter += 0.01f;
    pointLight.WorldPosition.x = sinf(counter) * 3.0f;
    pointLight.WorldPosition.y = 0.0f;
    pointLight.WorldPosition.z = cosf(counter) * 3.0f;
    pointLight.CalcLocalPosition(worldTransform);

    pLightingTech->SetPointLights(1, &pointLight);

    pLightingTech->SetMaterial(pMesh->GetMaterial());

    Vector3f CameraLocalPos3f = worldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    pMesh->Render();

    glutPostRedisplay();
    glutSwapBuffers();

    glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    for (int y = 0 ; y < WINDOW_HEIGHT; y++) {
        printf("%d ", y);
        for (int x = 0 ; x < WINDOW_WIDTH; x++) {
            unsigned char* p = &buffer[(y * WINDOW_WIDTH + x) * 3];
            if (p[0] != 0) {
                printf("%d, %d, %d ", p[0], p[1], p[2]);
            }
        }
        printf("\n");
    }

    stbi_write_png("test.png", WINDOW_WIDTH, WINDOW_HEIGHT, 3, buffer, WINDOW_WIDTH * 3);
}


#define ATTEN_STEP 0.01f

void TexturedCube::KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    switch (key) {
    case 'q':
    case 27:    // escape key code
        exit(0);

    case 'a':
        pointLight.Attenuation.Linear += ATTEN_STEP;
        break;

    case 'z':
        pointLight.Attenuation.Linear -= ATTEN_STEP;
        break;

    case 's':
        pointLight.Attenuation.Exp += ATTEN_STEP;
        break;

    case 'x':
        pointLight.Attenuation.Exp -= ATTEN_STEP;
        break;

    }

    printf("Linear %f Exp %f\n", pointLight.Attenuation.Linear, pointLight.Attenuation.Exp);

    pGameCamera->OnKeyboard(key);
}


void TexturedCube::SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGameCamera->OnKeyboard(key);
}


void TexturedCube::PassiveMouseCB(int x, int y)
{
    pGameCamera->OnMouse(x, y);
}


TexturedCube* pTexturedCube = NULL;


void RenderSceneCB()
{
    pTexturedCube->RenderSceneCB();
}


void KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    pTexturedCube->KeyboardCB(key, mouse_x, mouse_y);
}


void SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pTexturedCube->SpecialKeyboardCB(key, mouse_x, mouse_y);
}


void PassiveMouseCB(int x, int y)
{
    pTexturedCube->PassiveMouseCB(x, y);
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
    int win = glutCreateWindow("Textured Cube");
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

    pTexturedCube = new TexturedCube();

    if (!pTexturedCube->Init()) {
        return 1;
    }

    glutMainLoop();

    return 0;
}
