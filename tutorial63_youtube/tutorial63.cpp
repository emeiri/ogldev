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

    Tutorial 63 - HDR And Tone Mapping
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <random>

#include "demolition.h"
#include "demolition_base_gl_app.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class HDRToneMapping : public BaseGLApp {
public:
    HDRToneMapping() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 63 - HDR & Tone Mapping")
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
        m_pointLight.AmbientIntensity = 0.1f;
        m_pointLight.DiffuseIntensity = 1000.0f;
        m_pointLight.Attenuation.Constant = 0.1f;
        m_pointLight.Attenuation.Linear = 0.01f;
        m_pointLight.Attenuation.Exp = 0.001f;
        m_pointLight.WorldPosition = Vector3f(0.0f, 2.0f, 0.0f);
    }

    ~HDRToneMapping() {}


    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();

        m_pScene->SetClearColor(Vector4f(0.5f, 0.5f, 0.5f, 1.0f));

        //  m_pScene->SetCameraSpeed(0.1f);

       // m_pScene->GetDirLights().push_back(m_dirLights[0]);
       // m_pScene->GetDirLights().push_back(m_dirLights[1]);
        m_pScene->GetPointLights().push_back(m_pointLight);
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = false;
        m_pScene->GetConfig()->ControlShadowMapping(false);

        m_pRenderingSystem->SetScene(m_pScene);

        Model* pModel = m_pRenderingSystem->LoadModel("../Content/crytek_sponza/sponza.obj");        
        m_pScene->SetCamera(Vector3f(63.0f, 15.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.1f)); // for Sponze
        m_pSceneObject = m_pScene->CreateSceneObject(pModel);
        m_pSceneObject->SetScale(0.05f);
        m_pScene->AddToRenderList(m_pSceneObject);

        Model* pSphere = m_pRenderingSystem->LoadModel("../Content/sphere/scene.glb");
        //m_pScene->SetCamera(Vector3f(0.0f, 0.0f, -2.0f), Vector3f(0.0, 0.0f, 1.0f));

        //m_pScene->SetCamera(Vector3f(20.888552f, 16.027384f, -1.917199f), Vector3f(-0.898551f, -0.423240f, 0.116076f));
        
        //m_pScene->SetCamera(Vector3f(0.0f, 70.0f, -200.0f), Vector3f(0.0, -0.2f, 1.0f));        

       // Model* pModel = m_pRenderingSystem->LoadModel("../Content/teapot/teapot.obj");
   //     Model* pModel = m_pRenderingSystem->LoadModel("../Content/stanford_armadillo_pbr/scene.gltf");
       // Model* pModel = m_pRenderingSystem->LoadModel("../Content/rubber_duck/scene.gltf");
    //    Model* pModel = m_pRenderingSystem->LoadModel("../Content/jeep.obj");
         
    //    Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/McGuire/bistro/Exterior/exterior.obj");

        m_pLightObject = m_pScene->CreateSceneObject(pSphere);
        m_pLightObject->SetScale(1.0f);
        m_pScene->AddToRenderList(m_pLightObject);

        m_pBallObject = m_pScene->CreateSceneObject(pSphere);
        m_pBallObject->SetScale(0.1f);
        m_pScene->AddToRenderList(m_pBallObject);        

        m_pRenderingSystem->Execute();
    }


    void OnFrameChild(long long DeltaTimeMillis)
    {        
        //  if (m_pScene->GetDirLights().size() > 0) {
        //      m_pScene->GetDirLights()[0].WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        //  }

     //   m_pSceneObject->RotateBy(0.0f, 0.5f, 0.0f);
        
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

      //  m_pSceneObject->RotateBy(0.0f, 0.1f, 0.0f);
        //m_pSceneObject->ResetRotations();
      //  m_pSceneObject->PushRotation(Vector3f(180.0f, 0.0f, 0.0f));
     //   m_pSceneObject->PushRotation(Vector3f(0.0f, 0.0f, m_count));

        m_pScene->GetPointLights()[0].WorldPosition.x = sinf(m_count);
        m_pScene->GetPointLights()[0].WorldPosition.x = (cosf(m_count) + 1.0f) * 65.0f - 60.0f; // sponza
        m_pLightObject->SetPosition(m_pScene->GetPointLights()[0].WorldPosition);
        m_pBallObject->SetPosition(m_pScene->GetPointLights()[0].WorldPosition);

        //Vector3f LightPos = Vector3f(sinf(m_count) * 0.75f, 0.0f, cosf(m_count) * 0.75f);
        //m_pScene->GetPointLights()[0].WorldPosition = LightPos;
        //m_pBallObject->SetPosition(LightPos.x, LightPos.y, LightPos.z * -1.0f);
    }


protected:
    void OnFrameGUI()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::GetStyle().FontScaleMain = 1.5f;

        bool my_tool_active = false;
        ImGui::Begin("HDR & Tone Mapping", &my_tool_active, ImGuiWindowFlags_MenuBar);         

        m_pScene->ShowSceneGUI();

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
    HDRToneMapping demo;
    demo.Start();
}
