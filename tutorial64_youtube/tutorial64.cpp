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

    Tutorial 64 - PBR With GLTF2
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <random>

#include "demolition.h"
#include "demolition_base_gl_app.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440


class PBR_GLTF2 : public BaseGLApp {
public:
    PBR_GLTF2() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 64 - PBR using GLTF2")
    {
        m_dirLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.DiffuseIntensity = 2.0f;
        m_dirLight.AmbientIntensity = 0.1f;
    }

    ~PBR_GLTF2() {}


    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();
        m_pRenderingSystem->SetScene(m_pScene);
        m_pScene->SetClearColor(Vector4f(0.5f, 0.5f, 0.5f, 1.0f));

        SceneConfig* pConfig = m_pScene->GetConfig();

        pConfig->GetInfiniteGrid().Enabled = false;
        pConfig->ControlShadowMapping(false);

        m_pScene->GetDirLights().push_back(m_dirLight);

        //Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf");
        //Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/SciFiHelmet/glTF/SciFiHelmet.gltf");
         //Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/Sponza/glTF/Sponza.gltf");
          //Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/ABeautifulGame/glTF/ABeautifulGame.gltf");
//          Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/StainedGlassLamp/glTF/StainedGlassLamp.gltf");
    //      Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/ToyCar/glTF/ToyCar.gltf");
        Model* pModel = m_pRenderingSystem->LoadModel("../Content/DamagedHelmet/glTF/DamagedHelmet.gltf");
        int AOMap = m_pRenderingSystem->LoadTexture2D("../Content/DamagedHelmet/glTF/Default_AO.jpg");
        pModel->SetAmbientOcclusionMap(AOMap);

        Texture* pBRDF_LUT = new Texture(GL_TEXTURE_2D, "../Content/textures/brdfLUT.ktx");
        pBRDF_LUT->Load();
        pConfig->pBRDF_LUT = pBRDF_LUT;

        int IrrdianceMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_irradiance.ktx");
        pConfig->SetIrradianceMap(IrrdianceMap);

        int EnvMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_prefilter.ktx");
        pConfig->SetEnvMap(EnvMap);

        m_pSceneObject = m_pScene->CreateSceneObject(pModel);
        m_pScene->AddToRenderList(m_pSceneObject);

        m_pScene->SetCamera(Vector3f(0.0f, 0.0f, -3.0f), Vector3f(0.0f, 0.0f, 1.0f));

        m_pRenderingSystem->Execute();
    }


    void OnFrameChild(long long DeltaTimeMillis)
    {        
        m_pSceneObject->RotateBy(0.0f, 0.0f, 0.5f);
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
    DirectionalLight m_dirLight;
    SceneObject* m_pSceneObject = NULL;
};



int main(int argc, char* arg[])
{
    PBR_GLTF2 demo;
    demo.Start();
}
