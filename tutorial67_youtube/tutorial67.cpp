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

    Tutorial 67 - Projected Texture
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <random>

#include "demolition.h"
#include "demolition_base_gl_app.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440


class ProjectedTexture : public BaseGLApp {
public:
    ProjectedTexture() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 67 - Projected Texture")
    {
    }

    ~ProjectedTexture() {}


    void Start()
    {
        ModelLoadFlags flags;
        flags.ConvertToLeftHanded = true;
        m_pScene = m_pRenderingSystem->CreateScene("../Tutorial67_youtube/projected_texture_scene.glb", flags);
        m_pScene->SetCamera(Vector3f(0.0f, 15.0f, -50.0f), Vector3f(0.0f, -0.28f, 1.0f));
        m_pScene->GetCurrentCamera()->SetZ(0.1f, 1000.0f);
      //  m_pScene->GetDirLights()[0].AmbientIntensity = 1.0f;
        SceneConfig* pSceneConfig = m_pScene->GetConfig();
        
        m_pScene->SetClearColor(Vector4f(0.0f, 0.0f, 0.0f, 1.0f));

        pSceneConfig->ForcePBRDisabled(true);
        pSceneConfig->GetInfiniteGrid().Enabled = false;
        pSceneConfig->ControlShadowMapping(false);
        pSceneConfig->ControlGammaCorrection(true);
        pSceneConfig->ControlBloom(false);

        TextureConfig TexConfig = { WRAP_MODE_CLAMP_TO_BORDER };
        int ProjectedTexture = m_pRenderingSystem->LoadTexture2D("../Content/textures/bedexpstock-prison.png", &TexConfig);
        pSceneConfig->SetProjectedTexture(ProjectedTexture);

        m_pRenderingSystem->SetScene(m_pScene);

        m_pRenderingSystem->Execute();
    }


    void OnFrameChild(double DeltaTime)
    {        
        glm::vec3 projPos = glm::vec3(15.0f, 5.0f, 0.0f);

        glm::vec3 projAt = glm::vec3(0.0f, 0.0f, 0.0f);

        glm::vec3 projUp = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 projView = glm::lookAt(projPos, projAt, projUp);

        glm::mat4 projProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 30.0f);

        glm::mat4 bias = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f));
        bias = glm::scale(bias, glm::vec3(0.5f));

        glm::mat4 ProjectionMatrix = bias * projProj * projView;

        m_pScene->GetConfig()->SetProjectionMatrix(ProjectionMatrix);
    }


protected:
    void OnFrameGUI()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::GetStyle().FontScaleMain = 1.5f;

        m_pScene->ShowSceneGUI();

        // Rendering
        ImGui::Render();
        //   int display_w, display_h;
    //    glfwGetFramebufferSize(window, &display_w, &display_h);
    //    glViewport(0, 0, display_w, display_h);
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }


private:

    Scene* m_pScene = NULL;
};



int main(int argc, char* arg[])
{
    ProjectedTexture demo;
    demo.Start();
}
