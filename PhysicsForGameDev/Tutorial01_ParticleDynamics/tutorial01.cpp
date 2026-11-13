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

#define GROUND_HEIGHT 0.5f    // the box rests on the ground when its center is at this height

static void PhysicsUpdateListener(const void* pObject, const glm::vec3& Pos, const glm::quat& Orientation)
{
    //GLM_PRINT_VEC3("", Pos);
    SceneObject* pSceneObject = (SceneObject*)pObject;

    if (Pos.y >= GROUND_HEIGHT) {
        pSceneObject->SetPosition(Pos);
    }
}


class ParticleDynamics : public BaseGLApp {
public:
    ParticleDynamics() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Physics Tutorial 01 - Particle Dynamics")
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
        Model* pModel = m_pRenderingSystem->LoadModel("../Content/box.obj");
        //Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/ParticleDynamicsCarPaint/glTF/ParticleDynamicsCarPaint.gltf");
       // Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/glTF-Sample-Assets/Models/CesiumMan/glTF/CesiumMan.gltf");
       // Model* pModel = m_pRenderingSystem->LoadModel("../Content/glTF-Sample-Assets/ParticleDynamicsWicker/glTF/ParticleDynamicsWicker.gltf");
        int AOMap = m_pRenderingSystem->LoadTexture2D("../Content/DamagedHelmet/glTF/Default_AO.jpg");
        pModel->SetAmbientOcclusionMap(AOMap);

        Texture* pBRDF_LUT = new Texture(GL_TEXTURE_2D, "../Content/textures/brdfLUT.ktx");
        pBRDF_LUT->Load();
        pConfig->pBRDF_LUT = pBRDF_LUT;

        int IrrdianceMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_irradiance.ktx");
        pConfig->SetIrradianceMap(IrrdianceMap);

        int EnvMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_prefilter.ktx");
        pConfig->SetEnvMap(EnvMap);

       // m_pScene->GetConfig()->ControlSkybox(true);
       // m_pScene->LoadSkybox("../Content/textures/143_hdrmaps_com_free_10K_small.jpg");

        m_pBoxSceneObject = m_pScene->CreateSceneObject(pModel);
        m_pBoxSceneObject->SetPosition(m_startPos.x, m_startPos.y, m_startPos.z);
        m_pScene->AddToRenderList(m_pBoxSceneObject);

        m_pScene->SetCamera(Vector3f(0.0f, 7.0f, -25.0f), Vector3f(0.0f, 0.05f, 1.0f));

        // Gravity is applied as a global force on every physics step
        m_physicsSystem.Init(100, 0, PhysicsUpdateListener, Physics::GRAVITY);

        m_pPointMass = m_physicsSystem.AllocPointMass();
        m_pPointMass->Init(1.0f, m_startPos, glm::vec3(0.0f), m_pBoxSceneObject);

        m_pRenderingSystem->Execute();
    }


    void OnFrameChild(double DeltaTime)
    {
        m_physicsSystem.Update(DeltaTime);

        // When the box reaches the ground reset it back to the starting position
        if (m_pPointMass->GetPos().y <= GROUND_HEIGHT) {
            m_pPointMass->SetPos(m_startPos);
            m_pPointMass->SetLinearVelocity(glm::vec3(0.0f));
            m_pBoxSceneObject->SetPosition(m_startPos.x, m_startPos.y, m_startPos.z);
        }

        m_frameCount++;
    }

protected:

    void OnFrameGUI()
    {
        m_pScene->StartSceneGUI("Tutorial 01 - Particle Dynamics");
        m_pScene->EndSceneGUI();
    }

private:

    Scene* m_pScene = NULL;
    DirectionalLight m_dirLight;
    SceneObject* m_pBoxSceneObject = NULL;
    glm::vec3 m_startPos = glm::vec3(0.0f, 15.0f, 0.0f);
    Physics::System m_physicsSystem;
    Physics::PointMass* m_pPointMass = NULL;
    int m_frameCount = 0;
};



int main(int argc, char* arg[])
{
    ParticleDynamics demo;
    demo.Start();
}
