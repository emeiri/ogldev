/*

        Copyright 2025 Etay Meiri

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

#include "ogldev_util.h"
#include "ogldev_vertex_buffer.h"
#include "ogldev_base_app2.h"
#include "ogldev_infinite_grid.h"
#include "ogldev_glm_camera.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_phong_renderer.h"
#include "water_technique.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class Water : public OgldevBaseApp2
{
public:

    Water()
    {
        m_dirLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_dirLight.DiffuseIntensity = 2.2f;
        m_dirLight.AmbientIntensity = 1.5f;
    }


    virtual ~Water()
    {
    }


    void Init()
    {
        InitBaseApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Water");

        m_pGameCamera->SetPos(glm::vec3(0.0f, 0.5f, -1.0f));

        InitInfiniteGrid();

       // DefaultInitGUI();

        InitRenderer();

        if (!m_waterTech.Init()) {
            exit(1);
        }
		
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    }


    virtual void RenderSceneCB(float dt)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
        glm::mat4 VP = m_pGameCamera->GetVPMatrix();

        Matrix4f m;
        m.InitIdentity();

        m_waterTech.Render(VP, m_time);

        m_time += dt;

        m_infiniteGrid.Render(m_config, VP, m_pGameCamera->GetPos());
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

    void InitRenderer()
    {
        m_phongRenderer.InitPhongRenderer(LightingTechnique::SUBTECH_DEFAULT);
        m_phongRenderer.SetCamera(m_pGameCamera);
        m_phongRenderer.SetDirLight(m_dirLight);
        m_phongRenderer.SetWireframeLineWidth(1.0f);
        m_phongRenderer.SetWireframeColor(Vector4f(.0f, 0.0f, 1.0f, 1.0f));
    }

    InfiniteGrid m_infiniteGrid;
    InfiniteGridConfig m_config;
    PhongRenderer m_phongRenderer;
    DirectionalLight m_dirLight;
    WaterTechnique m_waterTech;
    float m_time = 0.0f;
};


int main(int argc, char** argv)
{
    Water* app = new Water();

    app->Init();

    app->Run();

    delete app;

    return 0;
}
