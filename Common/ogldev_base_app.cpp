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
*/

#include "ogldev_base_app.h"

static OgldevBaseApp* app = NULL;

OgldevBaseApp::OgldevBaseApp()
{
    if (app) {
        printf("OgldevBaseApp already initialized\n");
        exit(1);
    }

    app = this;
}


OgldevBaseApp::~OgldevBaseApp()
{
    SAFE_DELETE(m_pGameCamera);
}


void OgldevBaseApp::InitBaseApp()
{

}


void OgldevBaseApp::DefaultCreateWindow(int WindowWidth, int WindowHeight, const char* pWindowName)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;

    int major_ver = 0;
    int minor_ver = 0;
    bool is_full_screen = false;
    m_pWindow = glfw_init(major_ver, minor_ver, WindowWidth, WindowHeight, is_full_screen, pWindowName);

    glfwSetCursorPos(m_pWindow, WindowWidth / 2, WindowHeight / 2);
}


void OgldevBaseApp::DefaultInitGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);
}



void OgldevBaseApp::DefaultCreateCameraPers()
{
    if ((m_windowWidth == 0) || (m_windowHeight == 0)) {
        printf("Invalid window dims: width %d height %d\n", m_windowWidth, m_windowHeight);
        exit(1);
    }

    if (m_pGameCamera) {
        printf("Camera already initialized\n");
        exit(1);
    }

    Vector3f Pos(0.0f, 0.0f, 0.0f);
    Vector3f Target(0.0f, 0.0f, 1.0f);
    Vector3f Up(0.0, 1.0f, 0.0f);

    float FOV = 45.0f;
    float zNear = 1.0f;
    float zFar = 1000.0f;
    PersProjInfo persProjInfo = { FOV, (float)m_windowWidth, (float)m_windowHeight, zNear, zFar };

    m_pGameCamera = new BasicCamera(persProjInfo, Pos, Target, Up);
}


bool OgldevBaseApp::KeyboardCB(int Key, int Action, int Mods)
{
    bool Handled = true;

    if (Action == GLFW_PRESS) {
        switch (Key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwDestroyWindow(m_pWindow);
            glfwTerminate();
            exit(0);

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

        default:
            Handled = false;
        }
    }

    if (!Handled) {
        m_pGameCamera->OnKeyboard(Key);
    }

    return false;
}


void OgldevBaseApp::MouseButtonCB(int Button, int Action, int Mods, int x, int y)
{
}


void OgldevBaseApp::MouseMoveCB(int x, int y)
{
    if (!m_isPaused) {
        m_pGameCamera->OnMouse(x, y);
    }
}


static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    app->KeyboardCB(key, action, mods);
}


static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    app->MouseMoveCB((int)x, (int)y);
}


static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mods)
{
    double x, y;

    glfwGetCursorPos(window, &x, &y);

    app->MouseButtonCB(Button, Action, Mods, (int)x, (int)y);
}


void OgldevBaseApp::DefaultInitCallbacks()
{
    glfwSetKeyCallback(m_pWindow, KeyCallback);
    glfwSetCursorPosCallback(m_pWindow, CursorPosCallback);
    glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
}

void OgldevBaseApp::SetWindowShouldClose()
{
    glfwSetWindowShouldClose(m_pWindow, GLFW_TRUE);
}


void OgldevBaseApp::Run()
{
    float CurTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(m_pWindow)) {
        float Time = (float)glfwGetTime();
        float dt = Time - CurTime;
        RenderSceneCB(dt);
        CurTime = Time;
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }
}