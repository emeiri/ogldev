/*

        Copyright 2024 Etay Meiri

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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "demolition_base_gl_app.h"

BaseGLApp::BaseGLApp(int WindowWidth, int WindowHeight, const char* pWindowName)
{
    bool LoadBasicShapes = false;
    m_pRenderingSystem = RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM_GL, this, LoadBasicShapes);
    m_pWindow = (GLFWwindow*)m_pRenderingSystem->CreateWindow(WindowWidth, WindowHeight, pWindowName);
    InitGUI();
}


void BaseGLApp::InitGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);
}



void BaseGLApp::OnFrame(long long DeltaTimeMillis)
{
    OnFrameChild(DeltaTimeMillis);
}


void BaseGLApp::OnFrameEnd()
{
    if (m_showGui) {
        OnFrameGUI();
    }
}


bool BaseGLApp::OnKeyboard(int key, int action) 
{ 
    bool HandledByMe = true;

    switch (key) {
    case GLFW_KEY_SPACE:
        if (action == GLFW_PRESS) {
            m_showGui = !m_showGui;
        }
        break;

    case GLFW_KEY_Z:
        if (action == GLFW_PRESS) {
            m_isWireframe = !m_isWireframe;
            if (m_isWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
        break;

    case GLFW_KEY_KP_4:
        m_pRenderingSystem->GetCurrentCamera()->SetTarget(glm::vec3(-1.0f, 0.0f, 0.0f));
        m_pRenderingSystem->GetCurrentCamera()->SetUp(glm::vec3(0.0f, 1.0f, 0.0f));
        break;

    case GLFW_KEY_KP_6:
        m_pRenderingSystem->GetCurrentCamera()->SetTarget(glm::vec3(1.0f, 0.0f, 0.0f));
        m_pRenderingSystem->GetCurrentCamera()->SetUp(glm::vec3(0.0f, 1.0f, 0.0f));
        break;

    case GLFW_KEY_KP_8:
        m_pRenderingSystem->GetCurrentCamera()->SetTarget(glm::vec3(.0f, 0.0f, 1.0f));
        m_pRenderingSystem->GetCurrentCamera()->SetUp(glm::vec3(0.0f, 1.0f, 0.0f));
        break;

    case GLFW_KEY_KP_2:
        m_pRenderingSystem->GetCurrentCamera()->SetTarget(glm::vec3(.0f, 0.0f, -1.0f));
        m_pRenderingSystem->GetCurrentCamera()->SetUp(glm::vec3(0.0f, 1.0f, 0.0f));
        break;

   /* case GLFW_KEY_KP_7:
        m_pRenderingSystem->GetCurrentCamera()->SetTarget(Vector3f(.0f, 1.0f, 0.0f));
        m_pRenderingSystem->GetCurrentCamera()->SetUp(0.0f, 0.0f, -1.0f);
        break;*/

    case GLFW_KEY_R:
        m_pRenderingSystem->ReloadShaders();
        break;

    default:
        HandledByMe = false;
        break;
    }

    return HandledByMe;
}



bool BaseGLApp::OnMouseMove(int x, int y) 
{ 
    return !m_leftMousePressed;
}


bool BaseGLApp::OnMouseButton(int Button, int Action, int Mode, int x, int y)
{
    bool HandledByMe = true;

    switch (Button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        m_leftMousePressed = (Action == GLFW_PRESS);
        HandledByMe = m_leftMousePressed && m_showGui;
        break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
        m_midMousePressed = (Action == GLFW_PRESS);
       // m_pScene->GetConfig()->ControlPicking(m_midMousePressed);
        break;

    default:
        HandledByMe = false;
    }

    return HandledByMe;
}


void BaseGLApp::OnFrameGUI()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    Scene* pScene = m_pRenderingSystem->GetScene();

    bool my_tool_active = false;

    ImGui::Begin("Test", &my_tool_active, ImGuiWindowFlags_MenuBar);  // Create a window called "Hello, world!" and append into it.

   // GUIMenu();

   // GUICamera(pScene);

   // GUIScene(pScene);

    ImGui::CheckboxFlags("Enable Reflection/Refraction", &m_isRefRefractEnabled, 1);
    pScene->GetConfig()->ControlRefRefract(m_isRefRefractEnabled);

    ImGui::SliderFloat("Material/RefRefract Factor", &m_matRefRefractFactor, 0.0f, 1.0f);
    pScene->GetConfig()->SetMatRefRefractFactor(m_matRefRefractFactor);

    ImGui::SliderFloat("Index Of Refraction", &m_indexOfRefraction, 1.0f, 3.0f);
    pScene->GetConfig()->SetIndexOfRefraction(m_indexOfRefraction);

    ImGui::SliderFloat("Fresnel Power", &m_fresnelPower, 1.0f, 7.0f);
    pScene->GetConfig()->SetFresnelPower(m_fresnelPower);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    // Rendering
    ImGui::Render();
    //   int display_w, display_h;
//    glfwGetFramebufferSize(window, &display_w, &display_h);
//    glViewport(0, 0, display_w, display_h);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
