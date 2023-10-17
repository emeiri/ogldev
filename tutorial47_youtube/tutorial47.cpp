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

    Tutorial 47 - Introduction to Tessellation - Bezier Curve Rendering
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>


#include "ogldev_util.h"
#include "ogldev_basic_glfw_camera.h"
#include "ogldev_glfw.h"
#include "ogldev_bezier_curve_technique.h"
#include "ogldev_passthru_vec2_technique.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);

class VertexBuffer {
public:
    VertexBuffer()
    {

    }

    ~VertexBuffer()
    {

    }

    void Init(const std::vector<float>& Vertices)
    {        
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        glPatchParameteri(GL_PATCH_VERTICES, 4);
    }


    void Update(const std::vector<float>& Vertices)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }


    void Render(int topology_type)
    {
        if ((topology_type != GL_POINTS) && (topology_type != GL_PATCHES)) {
            printf("Invalid topology type 0x%x\n", topology_type);
            exit(1);
        }

        glBindVertexArray(m_vao);
        glDrawArrays(topology_type, 0, 4);
    }

private:
    GLuint m_vbo = -1;
    GLuint m_vao = -1;
};


class Tutorial47
{
public:

    Tutorial47()
    {    
    }


    virtual ~Tutorial47()
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

        glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
        glFrontFace(GL_CW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CLIP_DISTANCE0);
        glPointSize(10.0f);
        glLineWidth(10.0f);
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

        m_pGameCamera->OnRender();

        m_bezierCurveTech.Enable();
        m_bezierCurveTech.SetWVP(m_pGameCamera->GetViewProjMatrix());
        m_bezierCurveTech.SetNumSegments(m_numSegments);
        m_vertexBuffer.Render(GL_PATCHES);

        m_passThruTech.Enable();
        m_vertexBuffer.Render(GL_POINTS);
    }
    
    void PassiveMouseCB(int x, int y)
    {
        if (!m_isPaused) {
            m_pGameCamera->OnMouse(x, y);
        }
    }


#define STEP 0.01f

    void KeyboardCB(uint key, int state)
    {
        bool Handled = true;
        bool UpdateVertices = false;

        if (state == GLFW_PRESS) {
            switch (key) {
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
                glfwDestroyWindow(window);
                glfwTerminate();
                exit(0);

            case GLFW_KEY_P:
                m_isPaused = !m_isPaused;
                break;

            case GLFW_KEY_1:
                m_curVertex = 0;
                break;

            case GLFW_KEY_2:
                m_curVertex = 1;
                break;

            case GLFW_KEY_3:
                m_curVertex = 2;
                break;

            case GLFW_KEY_4:
                m_curVertex = 3;
                break;

            case GLFW_KEY_UP:
                m_vertices[m_curVertex * 2 + 1] += STEP;
                UpdateVertices = true;
                break;

            case GLFW_KEY_DOWN:
                m_vertices[m_curVertex * 2 + 1] -= STEP;
                UpdateVertices = true;
                break;

            case GLFW_KEY_LEFT:
                m_vertices[m_curVertex * 2] -= STEP;
                UpdateVertices = true;
                break;

            case GLFW_KEY_RIGHT:
                m_vertices[m_curVertex * 2] += STEP;
                UpdateVertices = true;
                break;

            case GLFW_KEY_A:
                m_numSegments++;
                break;

            case GLFW_KEY_Z:
                if (m_numSegments > 0) {
                    m_numSegments--;
                }
                break;

            default:
                Handled = false;
            }
        }

        if (UpdateVertices) {
            m_vertexBuffer.Update(m_vertices);
        }

        if (!Handled) {
            m_pGameCamera->OnKeyboard(key);
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
        window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Tutorial 47");

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
        Vector3f CameraPos = Vector3f(0.0f, 0.0f, -1.0f);
        Vector3f CameraTarget = Vector3f(0.0f, 0.f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);
	
        float c = 3.5f;
        OrthoProjInfo orthoProjinfo;
        orthoProjinfo.l = -0.4f * c;
        orthoProjinfo.r = 0.4f * c;
        orthoProjinfo.b = -0.3f * c;
        orthoProjinfo.t = 0.3f * c;
        orthoProjinfo.n = 0.1f;
        orthoProjinfo.f = 100.0f;
        orthoProjinfo.Width = WINDOW_WIDTH;
        orthoProjinfo.Height = WINDOW_HEIGHT;

        m_pGameCamera = new BasicCamera(orthoProjinfo, CameraPos, CameraTarget, Up);
        m_pGameCamera->SetSpeed(0.1f);
    }


    void InitShaders()
    {
        if (!m_bezierCurveTech.Init()) {
            printf("Error initializing the bezier curve technique\n");
            exit(1);
        }

        m_bezierCurveTech.Enable();

        m_bezierCurveTech.SetNumSegments(m_numSegments);
        m_bezierCurveTech.SetLineColor(1.0f, 1.0f, 0.5f, 1.0f);

        if (!m_passThruTech.Init()) {
            printf("Error initializing the passthru technique\n");
            exit(1);
        }

        m_passThruTech.Enable();

        m_passThruTech.SetColor(1.0f, 0.0f, 0.0f);
    }


    void InitMesh()
    {        
        m_vertexBuffer.Init(m_vertices);
    }

    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
	bool m_isPaused = false;
    VertexBuffer m_vertexBuffer;
    BezierCurveTechnique m_bezierCurveTech;
    PassthruVec2Technique m_passThruTech;
    std::vector<float> m_vertices = { -0.95f, -0.95f,     // X Y
                                      -0.85f, 0.95f,      // X Y
                                      0.5f, -0.95f,       // X Y
                                      0.95f, 0.95f };     // X Y
    int m_curVertex = 0;
    int m_numSegments = 50;
};

Tutorial47* app = NULL;

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
    app = new Tutorial47();

    app->Init();

    app->Run();

    delete app;

    return 0;
}
