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

    Ray Marching Demo
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "ogldev_util.h"
#include "Techniques\ogldev_ray_marching_technique.h"
#include "ogldev_vertex_buffer.h"
#include "ogldev_base_app.h"

#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000


class RayMarching : public OgldevBaseApp
{
public:

    RayMarching()
    {
    }


    virtual ~RayMarching()
    {
    }


    void Init()
    {
        DefaultCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Ray Marching");

        DefaultInitCallbacks();

        DefaultCreateCameraPers();        

        DefaultInitGUI();

        InitShaders();      

        m_rayMarchingTech.Enable();

        m_vertexBuffer.Init();

        glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
//        glLineWidth(5.0f);
    }


    virtual void RenderSceneCB()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_rayMarchingTech.SetCameraPos(m_pGameCamera->GetPos());
        m_vertexBuffer.Render();

        RenderGui();
    }


    void RenderGui()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Ray Marching Demo");

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_pWindow, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

#define STEP 0.01f

private:

    void InitShaders()
    {
        if (!m_rayMarchingTech.Init()) {
            printf("Error initializing the bezier curve technique\n");
            exit(1);
        }

        m_rayMarchingTech.Enable();
    }


    void InitMesh()
    {
        
    }

    FullScreenVB m_vertexBuffer;
    RayMarchingTechnique m_rayMarchingTech;
};


int main(int argc, char** argv)
{
    RayMarching* app = new RayMarching();

    app->Init();

    app->Run();

    delete app;

    return 0;
}
