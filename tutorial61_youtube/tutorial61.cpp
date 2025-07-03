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

    Tutorial 61 - Cloth Simulation
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "ogldev_util.h"
#include "ogldev_vertex_buffer.h"
#include "ogldev_base_app2.h"
#include "ogldev_infinite_grid.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_phong_renderer.h"
#include "cloth.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class Tutorial61 : public OgldevBaseApp2
{
public:

    Tutorial61()
    {
    }


    virtual ~Tutorial61()
    {
    }


    void Init()
    {
        InitBaseApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 61");

        m_pGameCamera->SetPos(glm::vec3(1.0f, -2.5f, -5.0f));

        InitInfiniteGrid();

       // DefaultInitGUI();

        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

        m_cloth.Init();
    }


    virtual void RenderSceneCB(float dt)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4& Projection = m_pGameCamera->GetProjMatrixGLM();
        glm::mat4 View = m_pGameCamera->GetViewMatrix();

       // m_infiniteGrid.Render(m_config, VP, m_pGameCamera->GetPos());
        glm::mat4 World = glm::mat4(1.0);
        m_cloth.Render(dt, World, View, Projection);
        //m_infiniteGrid.Render(m_config, View * Projection, m_pGameCamera->GetPos());
    }


    void RenderGui()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_pWindow, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    private:


    void InitInfiniteGrid()
    {
        m_infiniteGrid.Init(); 
    }

    InfiniteGrid m_infiniteGrid;
    InfiniteGridConfig m_config;
    Cloth m_cloth;
};


int main(int argc, char** argv)
{
    Tutorial61* app = new Tutorial61();

    app->Init();

    app->Run();

    delete app;

    return 0;
}
