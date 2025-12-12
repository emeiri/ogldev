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

    Physics Tutorial 01 - Particle Dynamics
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <random>

#include "demolition.h"
#include "demolition_base_gl_app.h"
#include "physics_system.h"

#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440

static void PhysicsUpdateListener(void* pObject, const glm::vec3& Pos)
{
    //GLM_PRINT_VEC3("", Pos);
    SceneObject* pSceneObject = (SceneObject*)pObject;

    if (Pos.y >= 0.5f) {
        pSceneObject->SetPosition(Pos);
    }
}


class ParticleDynamics : public BaseGLApp {
public:
    ParticleDynamics() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Physics Tutorial 02 - Particle Collision")
    {
        m_dirLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.DiffuseIntensity = 2.0f;
        m_dirLight.AmbientIntensity = 0.1f;
    }

    ~ParticleDynamics() {}


    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();
        m_pRenderingSystem->SetScene(m_pScene);
        m_pScene->SetClearColor(Vector4f(0.5f));

        SceneConfig* pConfig = m_pScene->GetConfig();

        pConfig->GetInfiniteGrid().Enabled = true;

        m_pScene->GetDirLights().push_back(m_dirLight);
  
       // Model* pModel = m_pRenderingSystem->LoadModel("../Content/porsche/porsche_no_ground.gltf");
        Model* pSphere1 = m_pRenderingSystem->LoadModel("../PhysicsForGameDev/Tutorial02_ParticleCollision/sphere1.obj");
        Model* pSphere2 = m_pRenderingSystem->LoadModel("../PhysicsForGameDev/Tutorial02_ParticleCollision/sphere2.obj");
        //Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/ParticleDynamicsCarPaint/glTF/ParticleDynamicsCarPaint.gltf");
       // Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/CesiumMan/glTF/CesiumMan.gltf");
       // Model* pModel = m_pRenderingSystem->LoadModel("../Content/glTF-Sample-Assets/ParticleDynamicsWicker/glTF/ParticleDynamicsWicker.gltf");
        
        /*int AOMap = m_pRenderingSystem->LoadTexture2D("../Content/DamagedHelmet/glTF/Default_AO.jpg");
        pModel->SetAmbientOcclusionMap(AOMap);

        Texture* pBRDF_LUT = new Texture(GL_TEXTURE_2D, "../Content/textures/brdfLUT.ktx");
        pBRDF_LUT->Load();
        pConfig->pBRDF_LUT = pBRDF_LUT;

        int IrrdianceMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_irradiance.ktx");
        pConfig->SetIrradianceMap(IrrdianceMap);

        int EnvMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_prefilter.ktx");
        pConfig->SetEnvMap(EnvMap);*/

       // m_pScene->GetConfig()->ControlSkybox(true);
       // m_pScene->LoadSkybox("../Content/textures/143_hdrmaps_com_free_10K_small.jpg");

        m_pSphere1 = m_pScene->CreateSceneObject(pSphere1);
        m_pSphere1->SetPosition(-10.0f, 1.0f, 15.0);
        m_pScene->AddToRenderList(m_pSphere1);

        m_pSphere2 = m_pScene->CreateSceneObject(pSphere2);
        m_pSphere2->SetPosition(0.0f, 1.0f, 5.0);
        m_pScene->AddToRenderList(m_pSphere2);

        m_pScene->SetCamera(Vector3f(0.0f, 20.0f, 1.0f), Vector3f(0.0f, -0.9f, 0.3f));

        m_physicsSystem.Init(100, PhysicsUpdateListener);

        m_pPointMass1 = m_physicsSystem.AllocPointMass();
        float RandomX = 1.0f;//RandomFloatRange(1.0f, 5.0f);
        float RandomZ = 0.0f;//RandomFloatRange(-0.5f, 0.5f);
        m_pPointMass1->Init(1.0f, m_pSphere1->GetGLMPos(), glm::vec3(RandomX, 0.0f, RandomZ), m_pSphere1);
        m_pPointMass1->SetBoundingRadius(1.0f);

        m_pPointMass2 = m_physicsSystem.AllocPointMass();
        m_pPointMass2->Init(1.0f, m_pSphere2->GetGLMPos(), glm::vec3(0.0f, 0.0f, 0.0f), m_pSphere2);
        m_pPointMass2->SetBoundingRadius(1.0f);

        m_pRenderingSystem->Execute();
    }


    void OnFrameChild(long long DeltaTimeMillis)
    {  
        m_physicsSystem.Update((int)DeltaTimeMillis);

        if ((m_pSphere1->GetPosition().x >= 10.0f) ||
            (m_pSphere1->GetPosition().z >= 40.0f)) {
            float RandomX = 1.0f;//RandomFloatRange(1.0f, 5.0f);
            float RandomZ = 0.0f;//RandomFloatRange(-0.5f, 0.5f);
            m_pPointMass1->Init(1.0f, glm::vec3(-10.0f, 1.0f, 15.0), glm::vec3(RandomX, 0.0f, RandomZ), m_pSphere1);
        }

        m_frameCount++;
    }


    virtual bool OnKeyboard(int key, int action)
    {
        bool ret = false;

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_PRESS)) {
            m_pPointMass2->Init(1.0f, glm::vec3(0.0f, 1.0f, 5.0), glm::vec3(0.0f, 0.0f, 3.0f), m_pSphere2);
            ret = true;
        } else {
            ret = BaseGLApp::OnKeyboard(key, action);
        }

        return ret;
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
    SceneObject* m_pSphere1 = NULL;
    SceneObject* m_pSphere2 = NULL;
    Physics::System m_physicsSystem;
    Physics::PointMass* m_pPointMass1 = NULL;
    Physics::PointMass* m_pPointMass2 = NULL;
    int m_frameCount = 0;
};



int main(int argc, char* arg[])
{
    ParticleDynamics demo;
    demo.Start();
}
