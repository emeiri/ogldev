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

    Tutorial 17 - Vertex Array Objects
*/

#include <stdio.h>
#include <string.h>

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_math_3d.h"
#include "ogldev_texture.h"
#include "camera.h"
#include "world_transform.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440


struct Vertex {
    Vector3f pos;
    Vector2f tex;

    Vertex() {}

    Vertex(const Vector3f& pos_, const Vector2f& tex_)
    {
        pos = pos_;
        tex = tex_;
    }
};


class Tutorial17
{
public:
    Tutorial17();
    ~Tutorial17();

    bool Init();

    void RenderSceneCB();
    void KeyboardCB(unsigned char key, int mouse_x, int mouse_y);
    void SpecialKeyboardCB(int key, int mouse_x, int mouse_y);
    void PassiveMouseCB(int x, int y);

private:

    void CreateCubeVAO();
    void CreatePyramidVAO();
    void CompileShaders();
    void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);

    GLuint CubeVAO = -1;
    GLuint CubeVBO = -1;
    GLuint CubeIBO = -1;

    GLuint PyramidVAO = -1;
    GLuint PyramidVBO = -1;
    GLuint PyramidIBO = -1;

    GLuint WVPLocation;
    GLuint SamplerLocation;
    Texture* pTexture = NULL;
    Camera* pGameCamera = NULL;
    WorldTrans CubeWorldTransform;
    PersProjInfo persProjInfo;
};


Tutorial17::Tutorial17()
{
    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    float FOV = 45.0f;
    float zNear = 1.0f;
    float zFar = 100.0f;

    persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };
}


Tutorial17::~Tutorial17()
{
    if (pTexture) {
        delete pTexture;
    }

    if (pGameCamera) {
        delete pGameCamera;
    }

    if (CubeVAO != -1) {
        glDeleteVertexArrays(1, &CubeVAO);
    }

    if (CubeVBO != -1) {
        glDeleteBuffers(1, &CubeVBO);
    }

    if (CubeIBO != -1) {
        glDeleteBuffers(1, &CubeIBO);
    }

    if (PyramidVAO != -1) {
        glDeleteVertexArrays(1, &PyramidVAO);
    }

    if (PyramidVBO != -1) {
        glDeleteBuffers(1, &PyramidVBO);
    }

    if (PyramidIBO != -1) {
        glDeleteBuffers(1, &PyramidIBO);
    }
}


bool Tutorial17::Init()
{
    CreateCubeVAO();
    CreatePyramidVAO();

    // Bind a default object
    glBindVertexArray(CubeVAO);

    CompileShaders();

    pTexture = new Texture(GL_TEXTURE_2D, "../Content/bricks.jpg");

    if (!pTexture->Load()) {
        return false;
    }

    pTexture->Bind(GL_TEXTURE0);
    glUniform1i(SamplerLocation, 0);

    Vector3f CameraPos(0.0f, 0.0f, -1.0f);
    Vector3f CameraTarget(0.0f, 0.0f, 1.0f);
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, CameraPos, CameraTarget, CameraUp);

    return true;
}


void Tutorial17::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT);

    pGameCamera->OnRender();

#ifdef _WIN64
    float YRotationAngle = 0.1f;
#else
    float YRotationAngle = 1.0f;
#endif

    CubeWorldTransform.SetPosition(0.0f, 0.0f, 2.0f);
    CubeWorldTransform.Rotate(0.0f, YRotationAngle, 0.0f);

    Matrix4f World = CubeWorldTransform.GetMatrix();

    Matrix4f View = pGameCamera->GetMatrix();

    Matrix4f Projection;
    Projection.InitPersProjTransform(persProjInfo);

    Matrix4f WVP = Projection * View * World;
    glUniformMatrix4fv(WVPLocation, 1, GL_TRUE, &WVP.m[0][0]);

    GLint CurrentVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &CurrentVAO);

    if (CurrentVAO == CubeVAO) {
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    } else {
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    }

    glutPostRedisplay();

    glutSwapBuffers();
}


void Tutorial17::KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    switch (key) {
    case 'q':
    case 27:    // escape key code
        exit(0);

    case '1':
        glBindVertexArray(CubeVAO);
        break;

    case '2':
        glBindVertexArray(PyramidVAO);
        break;
    }

    pGameCamera->OnKeyboard(key);
}


void Tutorial17::SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGameCamera->OnKeyboard(key);
}


void Tutorial17::PassiveMouseCB(int x, int y)
{
    pGameCamera->OnMouse(x, y);
}


void Tutorial17::CreateCubeVAO()
{
    glGenVertexArrays(1, &CubeVAO);
    glBindVertexArray(CubeVAO);

    Vertex Vertices[8];

    Vector2f t00 = Vector2f(0.0f, 0.0f);  // Bottom left
    Vector2f t01 = Vector2f(0.0f, 1.0f);  // Top left
    Vector2f t10 = Vector2f(1.0f, 0.0f);  // Bottom right
    Vector2f t11 = Vector2f(1.0f, 1.0f);  // Top right

    Vertices[0] = Vertex(Vector3f(0.5f, 0.5f, 0.5f), t00);
    Vertices[1] = Vertex(Vector3f(-0.5f, 0.5f, -0.5f), t01);
    Vertices[2] = Vertex(Vector3f(-0.5f, 0.5f, 0.5f), t10);
    Vertices[3] = Vertex(Vector3f(0.5f, -0.5f, -0.5f), t11);
    Vertices[4] = Vertex(Vector3f(-0.5f, -0.5f, -0.5f), t00);
    Vertices[5] = Vertex(Vector3f(0.5f, 0.5f, -0.5f), t10);
    Vertices[6] = Vertex(Vector3f(0.5f, -0.5f, 0.5f), t01);
    Vertices[7] = Vertex(Vector3f(-0.5f, -0.5f, 0.5f), t11);

    glGenBuffers(1, &CubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    // tex coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

    unsigned int Indices[] = {
                              0, 1, 2,
                              1, 3, 4,
                              5, 6, 3,
                              7, 3, 6,
                              2, 4, 7,
                              0, 7, 6,
                              0, 5, 1,
                              1, 5, 3,
                              5, 0, 6,
                              7, 4, 3,
                              2, 1, 4,
                              0, 2, 7
    };

    glGenBuffers(1, &CubeIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CubeIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void Tutorial17::CreatePyramidVAO()
{
    glGenVertexArrays(1, &PyramidVAO);
    glBindVertexArray(PyramidVAO);

    Vector2f t00  = Vector2f(0.0f, 0.0f);
    Vector2f t050 = Vector2f(0.5f, 0.0f);
    Vector2f t10  = Vector2f(1.0f, 0.0f);
    Vector2f t051 = Vector2f(0.5f, 1.0f);

    Vertex Vertices[4] = { Vertex(Vector3f(-1.0f, -1.0f, 0.5773f), t00),
                           Vertex(Vector3f(0.0f, -1.0f, -1.15475f), t050),
                           Vertex(Vector3f(1.0f, -1.0f, 0.5773f), t10),
                           Vertex(Vector3f(0.0f, 1.0f, 0.0f), t051) };

    glGenBuffers(1, &PyramidVBO);
    glBindBuffer(GL_ARRAY_BUFFER, PyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    // tex coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

    unsigned int Indices[] = { 0, 3, 1,
                               1, 3, 2,
                               2, 3, 0,
                               0, 1, 2 };

    glGenBuffers(1, &PyramidIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PyramidIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void Tutorial17::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(1);
    }

    const GLchar* p[1];
    p[0] = pShaderText;

    GLint Lengths[1];
    Lengths[0] = (GLint)strlen(pShaderText);

    glShaderSource(ShaderObj, 1, p, Lengths);

    glCompileShader(ShaderObj);

    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj);
}


void Tutorial17::CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    std::string vs, fs;

    if (!ReadFile("shader.vs", vs)) {
        exit(1);
    };

    AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

    if (!ReadFile("shader.fs", fs)) {
        exit(1);
    };

    AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram);

    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    WVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
    if (WVPLocation == -1) {
        printf("Error getting uniform location of 'gWVP'\n");
        exit(1);
    }

    SamplerLocation = glGetUniformLocation(ShaderProgram, "gSampler");
    if (SamplerLocation == -1) {
        printf("Error getting uniform location of 'gSampler'\n");
        exit(1);
    }

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);
}


Tutorial17* pTutorial17 = NULL;


void RenderSceneCB()
{
    pTutorial17->RenderSceneCB();
}


void KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    pTutorial17->KeyboardCB(key, mouse_x, mouse_y);
}


void SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pTutorial17->SpecialKeyboardCB(key, mouse_x, mouse_y);
}


void PassiveMouseCB(int x, int y)
{
    pTutorial17->PassiveMouseCB(x, y);
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
    int win = glutCreateWindow("Tutorial 17");
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

    pTutorial17 = new Tutorial17();

    if (!pTutorial17->Init()) {
        return 1;
    }

    glutMainLoop();

    return 0;
}
