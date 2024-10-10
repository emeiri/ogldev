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

    Tutorial 55 - GLM Camera
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "ogldev_util.h"
#include "ogldev_vertex_buffer.h"
#include "ogldev_base_app.h"
#include "ogldev_infinite_grid.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class Tutorial55 : public OgldevBaseApp
{
public:

    Tutorial55()
    {
    }


    virtual ~Tutorial55()
    {
    }


    void Init()
    {
        DefaultCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 55");

        DefaultInitCallbacks();

        InitCamera();

        InitInfiniteGrid();

     //   DefaultInitGUI();

        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
      //  glFrontFace(GL_CCW);
      //  glEnable(GL_CULL_FACE);
        //glEnable(GL_DEPTH_TEST);
    }


    virtual void RenderSceneCB()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Matrix4f VP = m_pGameCamera->GetViewProjMatrix();

        m_infiniteGrid.Render(m_config, VP, m_pGameCamera->GetPos());
       // RenderGui();
    }


    void RenderGui()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /*ImGui::Begin("Quad Tessellation");

        ImGui::SliderFloat("Outer Left Tessellation Level", &this->m_outerLevelLeft, 0.0f, 50.0f);
        ImGui::SliderFloat ("Outer Bottom Tessellation Level", &this->m_outerLevelBottom, 0.0f, 50.0f);
        ImGui::SliderFloat ("Outer Right Tessellation Level", &this->m_outerLevelRight, 0.0f, 50.0f);
        ImGui::SliderFloat("Outer Top Tessellation Level", &this->m_outerLevelTop, 0.0f, 50.0f);

        ImGui::SliderFloat("Inner Left/Right Tessellation Level", &this->m_innerLevelLeftRight, 0.0f, 50.0f);
        ImGui::SliderFloat("Inner Top/Bottom Tessellation Level", &this->m_innerLevelTopBottom, 0.0f, 50.0f);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();*/

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_pWindow, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

#define STEP 0.01f

    private:

    void InitCamera()
    {
        Vector3f Pos(0.0f, 0.0f, 0.0f);
        Vector3f Target(0.0f, 0.0f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        float FOV = 45.0f;
        float zNear = 1.0f;
        float zFar = 1000.0f;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

        m_pGameCamera = new BasicCamera(persProjInfo, Pos, Target, Up);
    }


    void InitInfiniteGrid()
    {
        m_infiniteGrid.Init(); 
    }

    InfiniteGrid m_infiniteGrid;
    InfiniteGridConfig m_config;
};


int main(int argc, char** argv)
{
    Tutorial55* app = new Tutorial55();

    app->Init();

    app->Run();

    delete app;

    return 0;
}
