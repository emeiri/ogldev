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

    Physics Tutorial 01 - 1D Kinematics
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <random>

#include "demolition.h"
#include "demolition_base_gl_app.h"
#include "physics_modeling.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440

static void PhysicsUpdateListener(void* pObject, const glm::vec3& Pos)
{
    //GLM_PRINT_VEC3("", Pos);
    SceneObject* pSceneObject = (SceneObject*)pObject;

    pSceneObject->SetPosition(Pos);
}


class ClearCoat : public BaseGLApp {
public:
    ClearCoat() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Physics Tutorial 01 - 1D Kinematics")
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

        m_pScene->GetDirLights().push_back(m_dirLight);
  
       // Model* pModel = m_pRenderingSystem->LoadModel("../Content/porsche/porsche_no_ground.gltf");
        Model* pModel = m_pRenderingSystem->LoadModel("../Content/box.obj");
        //Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/ClearCoatCarPaint/glTF/ClearCoatCarPaint.gltf");
       // Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/CesiumMan/glTF/CesiumMan.gltf");
       // Model* pModel = m_pRenderingSystem->LoadModel("../Content/glTF-Sample-Assets/ClearcoatWicker/glTF/ClearcoatWicker.gltf");
        int AOMap = m_pRenderingSystem->LoadTexture2D("../Content/DamagedHelmet/glTF/Default_AO.jpg");
        pModel->SetAmbientOcclusionMap(AOMap);

        Texture* pBRDF_LUT = new Texture(GL_TEXTURE_2D, "../Content/textures/brdfLUT.ktx");
        pBRDF_LUT->Load();
        pConfig->pBRDF_LUT = pBRDF_LUT;

        int IrrdianceMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_irradiance.ktx");
        pConfig->SetIrradianceMap(IrrdianceMap);

        int EnvMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_prefilter.ktx");
        pConfig->SetEnvMap(EnvMap);

        m_pCarSceneObject1 = m_pScene->CreateSceneObject(pModel);
        m_pScene->AddToRenderList(m_pCarSceneObject1);

        m_pCarSceneObject2 = m_pScene->CreateSceneObject(pModel);
        m_pCarSceneObject2->SetPosition(0.0f, 0.0f, 3.0);
        m_pScene->AddToRenderList(m_pCarSceneObject2);

        m_pCarSceneObject1->SetRotation(0.0f, -90.0f, 0.0f);

        m_pScene->SetCamera(Vector3f(0.0f, 4.0f, -8.0f), Vector3f(0.0f, -0.3f, 1.0f));

        m_physicsSystem.Init(100, PhysicsUpdateListener);

        m_pPointMass1 = m_physicsSystem.AllocPointMass();
        m_pPointMass1->Init(1.0f, m_pCarSceneObject1->GetGLMPos(), glm::vec3(0.1f, 0.0f, 0.0f), m_pCarSceneObject1);

        m_pPointMass2 = m_physicsSystem.AllocPointMass();
        m_pPointMass2->Init(1.0f, m_pCarSceneObject2->GetGLMPos(), glm::vec3(0.1f, 0.0f, 0.0f), m_pCarSceneObject2);

        m_pRenderingSystem->Execute();
    }


    void OnFrameChild(long long DeltaTimeMillis)
    {  
        m_physicsSystem.Update((int)DeltaTimeMillis);
        if (glm::length(m_pPointMass2->m_linearVelocity) > 1.0f) {
            m_pPointMass2->ResetSumForces();
        }
      //  m_pCarSceneObject1->RotateBy(0.0f, 0.5f, 0.0f);
      //  m_pCarSceneObject1->TranslateBy(-0.7f * (float)DeltaTimeMillis/1000.0f, 0.0f, 0.0f);
        m_frameCount++;
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
    SceneObject* m_pCarSceneObject1 = NULL;
    SceneObject* m_pCarSceneObject2 = NULL;
    Physics::System m_physicsSystem;
    Physics::PointMass* m_pPointMass1 = NULL;
    Physics::PointMass* m_pPointMass2 = NULL;
    int m_frameCount = 0;
};



int main(int argc, char* arg[])
{
    ClearCoat demo;
    demo.Start();
}
