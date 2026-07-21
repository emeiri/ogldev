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

    Tutorial 66 - Bloom
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <random>

#include "demolition.h"
#include "demolition_base_gl_app.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440


class Bloom : public BaseGLApp {
public:
    Bloom() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 66 - Bloom")
    {
        //  m_dirLight.WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_dirLights[0].WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_dirLights[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLights[0].DiffuseIntensity = 2.0f;
        m_dirLights[0].AmbientIntensity = 0.1f;

        m_dirLights[1].WorldDirection = Vector3f(-1.0f, -1.0f, 0.0f);
        m_dirLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLights[1].DiffuseIntensity = 2.0f;
        m_dirLights[1].AmbientIntensity = 0.0f;

        //m_pointLight.Color = Vector3f(1.0f, 0.0f, 0.0f);
        m_pointLight.Color = Vector3f(1.0f);
        m_pointLight.AmbientIntensity = 0.2f;
        m_pointLight.DiffuseIntensity = 0.8f;
        m_pointLight.Attenuation.Constant = 0.1f;
        m_pointLight.Attenuation.Linear = 0.01f;
        m_pointLight.Attenuation.Exp = 0.001f;
        m_pointLight.WorldPosition = Vector3f(0.0f, 0.5f, -5.0f);
    }

    ~Bloom() {}


    void Start()
    {
        ModelLoadFlags flags;
        flags.ConvertToLeftHanded = true;
        m_pScene = m_pRenderingSystem->CreateScene("../Tutorial66_youtube/bloom_scene.glb", flags);
        m_pScene->SetCamera(Vector3f(0.0f, 3.0f, -18.0f), Vector3f(0.0f, -0.08f, 1.0f));
        m_pScene->GetConfig()->SetLuminanceThreshold(0.5f);

        m_pScene->SetClearColor(Vector4f(0.0f, 0.0f, 0.0f, 1.0f));

       // m_pScene->GetDirLights().push_back(m_dirLights[0]);
       // m_pScene->GetDirLights().push_back(m_dirLights[1]);
      //  m_pScene->GetPointLights().push_back(m_pointLight);
        m_pScene->GetConfig()->ForcePBRDisabled(true);
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = false;
        m_pScene->GetConfig()->ControlShadowMapping(false);
        m_pScene->GetConfig()->ControlBloom(true);
        m_pScene->GetConfig()->ControlGammaCorrection(true);

        m_pRenderingSystem->SetScene(m_pScene);

        m_pRenderingSystem->Execute();
    }


    void OnFrame(double DeltaTime)
    {        
        m_count += 0.01f;

        if (m_pScene->GetPickedSceneObject()) {
            m_pickedObject = m_pScene->GetPickedSceneObject();
            m_pickedObject->SetColorMod(2.0f, 1.0f, 1.0f);
        }
        else {
            if (m_pickedObject) {
                m_pickedObject->SetColorMod(1.0f, 1.0f, 1.0f);
                m_pickedObject = NULL;
            }
        }

        m_pScene->GetPointLights()[0].WorldPosition.x = sinf(m_count);
        m_pScene->GetPointLights()[0].WorldPosition.z = cosf(m_count);
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

    float m_count = 0.0f;
    Scene* m_pScene = NULL;
    DirectionalLight m_dirLights[2];
    PointLight m_pointLight;
    SceneObject* m_pickedObject = NULL;
    SceneObject* m_pSceneObject = NULL;    
    SceneObject* m_pLightObject = NULL;
    SceneObject* m_pBallObject = NULL;
    int m_enableShadowMapping = 1;
};



int main(int argc, char* arg[])
{
    Bloom demo;
    demo.Start();
}
