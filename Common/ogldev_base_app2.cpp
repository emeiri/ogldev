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

#include "ogldev_base_app2.h"

static OgldevBaseApp2* app = NULL;

OgldevBaseApp2::OgldevBaseApp2()
{
    if (app) {
        printf("OgldevBaseApp2 already initialized\n");
        exit(1);
    }

    app = this;
}


OgldevBaseApp2::~OgldevBaseApp2()
{
    SAFE_DELETE(m_pGameCamera);
}


void OgldevBaseApp2::InitBaseApp(int WindowWidth, int WindowHeight, const char* pWindowName)
{
    DefaultCreateWindow(WindowWidth, WindowHeight, pWindowName);

    DefaultCreateCameraPers();

    DefaultInitCallbacks();

    DefaultInitGUI();
}


void OgldevBaseApp2::DefaultCreateWindow(int WindowWidth, int WindowHeight, const char* pWindowName)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;

    int major_ver = 0;
    int minor_ver = 0;
    bool is_full_screen = false;
    m_pWindow = glfw_init(major_ver, minor_ver, WindowWidth, WindowHeight, is_full_screen, pWindowName);

    glfwSetCursorPos(m_pWindow, WindowWidth / 2, WindowHeight / 2);
}


void OgldevBaseApp2::DefaultInitGUI()
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


void OgldevBaseApp2::DefaultCreateCameraPers()
{
    float FOV = 45.0f;
    float zNear = 1.0f;
    float zFar = 1000.0f;

    DefaultCreateCameraPers(FOV, zNear, zFar);
}


void OgldevBaseApp2::DefaultCreateCameraPers(float FOV, float zNear, float zFar)
{
    if ((m_windowWidth == 0) || (m_windowHeight == 0)) {
        printf("Invalid window dims: width %d height %d\n", m_windowWidth, m_windowHeight);
        exit(1);
    }

    if (m_pGameCamera) {
        printf("Camera already initialized\n");
        exit(1);
    }

    PersProjInfo persProjInfo = { FOV, (float)m_windowWidth, (float)m_windowHeight,
                                  zNear, zFar };

    glm::vec3 Pos(0.0f, 0.0f, 0.0f);
    glm::vec3 Target(0.0f, 0.0f, 1.0f);
    glm::vec3 Up(0.0, 1.0f, 0.0f);

    m_pGameCamera = new GLMCameraFirstPerson(Pos, Target, Up, persProjInfo);
}


bool OgldevBaseApp2::KeyboardCB(int Key, int Action, int Mods)
{
    bool Handled = true;
    
    switch (Key) {
    case GLFW_KEY_C:
        if (Action == GLFW_PRESS) {
            m_pGameCamera->Print();
        }
        break;
    case GLFW_KEY_ESCAPE:
    case GLFW_KEY_Q:
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
        exit(0);

    case GLFW_KEY_P:
        if (Action == GLFW_PRESS) {
            m_isPaused = !m_isPaused;
        }
        break;

    case GLFW_KEY_Z:
        if (Action == GLFW_PRESS) {
            m_isWireframe = !m_isWireframe;

            if (m_isWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
        break;

    case GLFW_KEY_SPACE:
        if (Action == GLFW_PRESS) {
            RenderGui();
        }
        break;

    default:
        Handled = false;
    }

    if (!Handled) {
        Handled = GLFWCameraHandler(m_pGameCamera->m_movement, Key, Action, Mods);
    }

    return Handled;
}


void OgldevBaseApp2::MouseButtonCB(int Button, int Action, int Mods, int x, int y)
{
    m_pGameCamera->HandleMouseButton(Button, Action, Mods);
   // m_pGameCamera->m_mouseState.m_buttonPressed = (Action == GLFW_PRESS);
}


void OgldevBaseApp2::MouseMoveCB(int x, int y)
{
    if (!m_isPaused) {
        m_pGameCamera->SetMousePos((float)x, (float)y);
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


static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode)
{
    double x, y;

    glfwGetCursorPos(window, &x, &y);

    app->MouseButtonCB(Button, Action, Mode, (int)x, (int)y);
}


void OgldevBaseApp2::DefaultInitCallbacks()
{
    if (!m_pGameCamera) {
        printf("Make sure to initialize the camera before the callbacks!\n");
        exit(1);
    }

    glfwSetKeyCallback(m_pWindow, KeyCallback);
    glfwSetCursorPosCallback(m_pWindow, CursorPosCallback);
    glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
}

void OgldevBaseApp2::SetWindowShouldClose()
{
    glfwSetWindowShouldClose(m_pWindow, GLFW_TRUE);
}


void OgldevBaseApp2::Run()
{
    float CurTime = (float)glfwGetTime();
    float FpsTime = 0.0f;
    int fps = 0;

    while (!glfwWindowShouldClose(m_pWindow)) {
        float Time = (float)glfwGetTime();
        float dt = Time - CurTime;
        m_pGameCamera->Update(dt);
        RenderSceneCB(dt);
        CurTime = Time;
        FpsTime += dt;
        if (FpsTime >= 1.0f) {
            printf("FPS %d\n", fps);
            FpsTime = 0.0f;
            fps = 0;
        }
        fps++;
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }
}