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

    DemoLITION - Grid demo
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "demolition.h"
#include "demolition_base_gl_app.h"


#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class GridDemo : public BaseGLApp {
public:
    GridDemo() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Infinite Grid")
    {
      //  m_dirLight.WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_dirLight.WorldDirection = Vector3f(0.0f, -1.0f, -1.0f);
        m_dirLight.DiffuseIntensity = 1.0f;

        m_pointLight.WorldPosition = Vector3f(0.25f, 0.25f, 0.0f);
     //  m_pointLight.WorldPosition = Vector3f(1.0f, 0.0f, -1.0f);
        m_pointLight.DiffuseIntensity = 2.0f;
        m_pointLight.AmbientIntensity = 0.1f;
    }

    ~GridDemo() {}

    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();

        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = true;

        /* DirectionalLight l;
         l.WorldDirection = Vector3f(1.0f, -0.5f, 0.0f);
         l.DiffuseIntensity = 0.5f;
         l.AmbientIntensity = 0.1f;
         pScene->GetDirLights().push_back(l);*/

     //   m_pScene->SetCamera(Vector3f(0.0f, 1.0f, -2.5f), Vector3f(0.000823f, -0.331338f, 0.943512f));
     //   m_pScene->SetCamera(Vector3f(-2.5f, 2.0f, -2.5f), Vector3f(0.66f, -0.5f, 0.66f));
     m_pScene->SetCamera(Vector3f(0.0f, 0.75f, -2.5f), Vector3f(0.0f, -0.2f, 1.0f));
   // 
   //     m_pScene->SetCamera(Vector3f(0.0f, 5.0f, 0.5f), Vector3f(-0.0f, -1.0f, 0.5f));

        m_pScene->SetCameraSpeed(0.1f);

        //m_pScene->GetDirLights().push_back(m_dirLight);
        m_pScene->GetPointLights().push_back(m_pointLight);

        m_pRenderingSystem->SetScene(m_pScene);

        m_pRenderingSystem->Execute();
    }


    void OnFrame(long long DeltaTimeMillis)
    {
        BaseGLApp::OnFrame(DeltaTimeMillis);

      //  m_pScene->SetCamera(Vector3f(0.0f, 0.05f, -2.5f), Vector3f(0.0f, -0.1f, 1.0f));
        //  m_pScene->GetDirLights()[0].WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_count += 0.01f;
        //m_pSceneObject->PushRotation(Vector3f(0.0f, 90.0f, 0.0f));
       // m_pScene->SetCamera(Vector3f(0.0f, 0.05f, -2.5f), Vector3f(0.0f, 0.1f * sinf(m_count) - 0.15f, 1.0f));
//        m_pScene->SetCamera(Vector3f(0.0f, 0.75f, m_count), Vector3f(0.0f, -0.2f, 1.0f));

        m_pScene->GetPointLights()[0].WorldPosition.x = sinf(m_count);
        m_pScene->GetPointLights()[0].WorldPosition.z = cosf(m_count);

        if (m_showGui) {
            RenderGUI();
        }
    }


    void RenderGUI()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool my_tool_active = false;

        ImGui::Begin("Test", &my_tool_active, ImGuiWindowFlags_MenuBar);      

        ImGui::SliderFloat("Grid Cell Size", &this->m_gridCellSize, 0.01f, 0.2f);
        m_pScene->GetConfig()->GetInfiniteGrid().CellSize = m_gridCellSize;
        
        ImGui::End();

        // Rendering
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }


    bool OnMouseMove(int x, int y)
    {
        if (m_showGui) {
            return true;
        } else {
            return BaseGLApp::OnMouseMove(x, y);
        }
    }


    bool OnMouseButton(int Button, int Action, int Mode, int x, int y)
    {
        if (m_showGui) {
            return true;
        } else {
            return BaseGLApp::OnMouseButton(Button, Action, Mode, x, y);
        }
    }
    

    bool OnKeyboard(int key, int action)
    {
        bool HandledByMe = true;

        switch (key) {
        case GLFW_KEY_SPACE:
            if (action == GLFW_PRESS) {
                m_showGui = !m_showGui;
            }
            break;

        default:
            HandledByMe = false;
        }
            
        if (HandledByMe) {
            return true;
        } else {
            return BaseGLApp::OnKeyboard(key, action);
        }            
    }

private:
    float m_count = 0.0f;
    Scene* m_pScene = NULL;
    DirectionalLight m_dirLight;
    PointLight m_pointLight;
    float m_gridCellSize = 0.025f;
    bool m_showGui = false;
};



void test_grid()
{
    GridDemo game;

    game.Start();
}
