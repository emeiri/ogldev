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

    Tutorial 56 - Cubemaps
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "ogldev_util.h"
#include "ogldev_vertex_buffer.h"
#include "ogldev_base_app2.h"
#include "ogldev_infinite_grid.h"
#include "ogldev_skybox.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class Tutorial56 : public OgldevBaseApp2
{
public:

    Tutorial56() {}

    virtual ~Tutorial56() {}

    void Init();

    virtual void RenderSceneCB(float dt);

    virtual void RenderGui();

private:

    SkyBox m_skybox;
};


void Tutorial56::Init()
{
    InitBaseApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 56");

    m_skybox.Init("../Content/textures/piazza_bologni_1k.hdr");
}


void Tutorial56::RenderSceneCB(float dt)
{	
    glm::mat4 VP = m_pGameCamera->GetVPMatrixNoTranslate();

    if (m_isPaused) {
        RenderGui();
    } else {
        m_skybox.Render(VP);
    }    
}


void Tutorial56::RenderGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Rendering
   // ImGui::Render();
   // int display_w, display_h;
   // glfwGetFramebufferSize(m_pWindow, &display_w, &display_h);
   // glViewport(0, 0, display_w, display_h);

   // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    bool my_tool_active = false;

    ImGui::Begin("Test", &my_tool_active, ImGuiWindowFlags_MenuBar);                          // Create a window called "Hello, world!" and append into it.

  //  GUIMenu();

  //  GUICamera(pScene);

  //  GUIScene(pScene);

    // ImGui::SliderFloat("Max height", &this->m_maxHeight, 0.0f, 1000.0f);
    // ImGui::SliderFloat("Terrain roughness", &this->m_roughness, 0.0f, 5.0f);

    // ImGui::SliderFloat("Height0", &Height0, 0.0f, 64.0f);
    //  ImGui::SliderFloat("Height1", &Height1, 64.0f, 128.0f);
    //   ImGui::SliderFloat("Height2", &Height2, 128.0f, 192.0f);
    //  ImGui::SliderFloat("Height3", &Height3, 192.0f, 256.0f);

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


int main(int argc, char** argv)
{
    Tutorial56* app = new Tutorial56();

    app->Init();

    app->Run();

    delete app;

    return 0;
}
