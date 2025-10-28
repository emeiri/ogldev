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

    Tutorial 65 - ClearCoat
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <random>

#include "demolition.h"
#include "demolition_base_gl_app.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440


class ClearCoat : public BaseGLApp {
public:
    ClearCoat() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 65 - Clear Coat")
    {
        m_dirLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.DiffuseIntensity = 2.0f;
        m_dirLight.AmbientIntensity = 0.1f;
    }

    ~ClearCoat() {}


    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();
        m_pRenderingSystem->SetScene(m_pScene);
        m_pScene->SetClearColor(Vector4f(0.5f));

        SceneConfig* pConfig = m_pScene->GetConfig();

        pConfig->GetInfiniteGrid().Enabled = true;
        pConfig->ControlShadowMapping(false);

        m_pScene->GetDirLights().push_back(m_dirLight);
  
        //Model* pModel = m_pRenderingSystem->LoadModel("../Content/old_kings_head/scene.gltf");
        //Model* pModel = m_pRenderingSystem->LoadModel("../Content/venice_well_head/scene.gltf");
        //Model* pModel = m_pRenderingSystem->LoadModel("../Content/vintage_cabinet_01/vintage_cabinet_01_4k.gltf");
        //Model* pModel = m_pRenderingSystem->LoadModel("../Content/porsche/scene.gltf");
        //Model* pModel = m_pRenderingSystem->LoadModel("../Content/santa_conga_freebiexmass/scene.gltf");
        //Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/ClearCoatCarPaint/glTF/ClearCoatCarPaint.gltf");
       // Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/CesiumMan/glTF/CesiumMan.gltf");
        Model* pModel = m_pRenderingSystem->LoadModel("../Content/glTF-Sample-Assets/ClearcoatWicker/glTF/ClearcoatWicker.gltf");
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
       // m_pSceneObject->SetScale(0.1f);
        m_pScene->AddToRenderList(m_pSceneObject);
      //  m_pSceneObject->SetRotation(180.0f, 0.0f, 0.0f);

        m_pScene->SetCamera(Vector3f(0.0f, 0.8f, -2.0f), Vector3f(0.0f, -0.25f, 1.0f));

        m_pRenderingSystem->Execute();
    }


    void OnFrameChild(long long DeltaTimeMillis)
    {        
        m_pSceneObject->RotateBy(0.0f, 0.5f, 0.0f);
      //  m_pSceneObject->TranslateBy(-0.7f * (float)DeltaTimeMillis/1000.0f, 0.0f, 0.0f);
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
    ClearCoat demo;
    demo.Start();
}
