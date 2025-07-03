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

    Tutorial 62 - Screen Space Ambient Occlusion
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <random>

#include "demolition.h"
#include "demolition_base_gl_app.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

static void CreateRandomOffsetArray()
{
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;

    printf("const vec3 offsets[64] = vec3[64](\n");

    for (unsigned int i = 0; i < 64; ++i) {
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator)
        );
        sample = glm::normalize(sample);
        sample *= randomFloats(generator); // bias toward center
        // Scale samples so more are near the origin
        float scale = float(i) / 64.0f;
        scale = glm::mix(0.1f, 1.0f, scale * scale); // quadratic distribution
        sample *= scale;

        printf("    vec3(%f, %f, %f),\n", sample[0], sample[1], sample[2]);
    }

    printf(");");
}


class SSAODemo : public BaseGLApp {
public:
    SSAODemo() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 62 - SSAO")
    {
        //  m_dirLight.WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_dirLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_dirLight.DiffuseIntensity = 0.9f;
        m_dirLight.AmbientIntensity = 0.1f;
    }

    ~SSAODemo() {}


    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();

        m_pScene->SetClearColor(Vector4f(1.0f, 0.0f, 1.0f, 1.0f));

        //  m_pScene->SetCameraSpeed(0.1f);

        m_pScene->GetDirLights().push_back(m_dirLight);
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = false;
        m_pScene->GetConfig()->ControlShadowMapping(false);

        m_pRenderingSystem->SetScene(m_pScene);

        m_pScene->SetCamera(Vector3f(63.0f, 19.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.1f)); // for Sponze
        //m_pScene->SetCamera(Vector3f(0.0f, 70.0f, -200.0f), Vector3f(0.0, -0.2f, 1.0f));

     //   Model* pModel = m_pRenderingSystem->LoadModel("../Content/teapot/teapot.obj");
   //     Model* pModel = m_pRenderingSystem->LoadModel("../Content/stanford_armadillo_pbr/scene.gltf");
       // Model* pModel = m_pRenderingSystem->LoadModel("../Content/rubber_duck/scene.gltf");
    //    Model* pModel = m_pRenderingSystem->LoadModel("../Content/jeep.obj");
         Model* pModel = m_pRenderingSystem->LoadModel("../Content/crytek_sponza/sponza.obj");
    //    Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/McGuire/bistro/Exterior/exterior.obj");
        m_pSceneObject = m_pScene->CreateSceneObject(pModel);
        m_pSceneObject->SetScale(0.05f);
        m_pScene->AddToRenderList(m_pSceneObject);

        m_pRenderingSystem->Execute();
    }


    void OnFrameChild(long long DeltaTimeMillis)
    {        
        //  if (m_pScene->GetDirLights().size() > 0) {
        //      m_pScene->GetDirLights()[0].WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        //  }

     //   m_pSceneObject->RotateBy(0.0f, 0.5f, 0.0f);
        
        m_count += 1.0f;

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

      //  m_pSceneObject->RotateBy(0.0f, 0.1f, 0.0f);
        //m_pSceneObject->ResetRotations();
      //  m_pSceneObject->PushRotation(Vector3f(180.0f, 0.0f, 0.0f));
     //   m_pSceneObject->PushRotation(Vector3f(0.0f, 0.0f, m_count));

         //   m_pScene->GetPointLights()[0].WorldPosition.x = sinf(m_count);
          //  m_pScene->GetPointLights()[0].WorldPosition.z = cosf(m_count);
    }


protected:
    void OnFrameGUI()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool my_tool_active = false;

        ImGui::Begin("SSAO", &my_tool_active, ImGuiWindowFlags_MenuBar); 

        m_pScene->ShowSSAOGUI();

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


private:

    float m_count = 0.0f;
    Scene* m_pScene = NULL;
    DirectionalLight m_dirLight;
    SceneObject* m_pickedObject = NULL;
    SceneObject* m_pSceneObject = NULL;    
    int m_enableShadowMapping = 1;
};



int main(int argc, char* arg[])
{
    SSAODemo demo;
    demo.Start();
}
