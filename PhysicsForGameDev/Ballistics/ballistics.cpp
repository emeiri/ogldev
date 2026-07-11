/*

        Copyright 2026 Etay Meiri

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

    Physics Ballistics Demo
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

static void PhysicsUpdateListener(const void* pObject, 
                                  const glm::vec3& Pos, 
                                  const glm::quat& Orientation)
{    
    SceneObject* pSceneObject = (SceneObject*)pObject;

//    printf("Update listener object %p\n", pObject);
  //  GLM_PRINT_VEC3("Pos", Pos);
    //GLM_PRINT_QUAT("Orientation ", Orientation);

    pSceneObject->SetPosition(Pos);
    pSceneObject->SetQuaternion(Orientation);
}

enum BallisticType {
    BALLISTIC_PISTOL,
    BALLISTIC_ARTILLERY,
    BALLISTIC_FIREBALL,
    BALLISTIC_RIFLE
};

float PistolMass = 0.01f;
glm::vec3 PistolForce = glm::vec3(20.0f, 3.0f, 0.0f);

float ArtilleryMass = 1.0f;
glm::vec3 ArtilleryForce = glm::vec3(800.0f, 800.0f, 0.0f);

float FireballMass = 0.5f;
glm::vec3 FireballForce = glm::vec3(200.0f, 100.0f, 0.0f);
glm::vec3 FireballAccel = glm::vec3(0.0f, 4.0f, 0.0f);

class ParticleDynamics : public BaseGLApp {

public:

    ParticleDynamics() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Physics Ballistics Demo")
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
        m_pScene->SetCamera(Vector3f(0.0f, 4.0f, 0.0f), Vector3f(0.0f, 0.05f, 1.0f));

        SceneConfig* pConfig = m_pScene->GetConfig();
        pConfig->GetInfiniteGrid().Enabled = true;

        m_pScene->GetDirLights().push_back(m_dirLight);
  
        Model* pModel = m_pRenderingSystem->LoadModel("../PhysicsForGameDev/Tutorial02_ParticleCollision/sphere1.obj");
        
       // pConfig->ControlSkybox(true);
     //   m_pScene->LoadSkybox("../Content/textures/143_hdrmaps_com_free_10K_small.jpg");

        CreateSceneObjects(pModel);

        m_physicsSystem.Init(100, 100, PhysicsUpdateListener, Physics::GRAVITY);

        CreatePointMasses();

        m_pRenderingSystem->Execute();
    }


    void CreateSceneObjects(Model* pModel)
    {
        m_pSceneObject = m_pScene->CreateSceneObject(pModel);
        m_pSceneObject->SetPosition(m_pos);
        m_pSceneObject->SetScale(0.2f);
        m_pScene->AddToRenderList(m_pSceneObject);
    }


    void CreatePointMasses()
    {
        m_pPointMass = m_physicsSystem.AllocPointMass();
        m_pPointMass->Init(PistolMass, m_pSceneObject->GetGLMPos(), PistolForce, m_pSceneObject);
    }


    void OnFrameChild(double DeltaTime)
    { 
        if (m_pPointMass->GetPos().y < 0.0f) {
           // printf("Initializing\n");
            switch (m_ballisticType) {
            case BALLISTIC_PISTOL:
                m_pPointMass->Init(PistolMass, m_pos, PistolForce, m_pSceneObject);
                m_fireballStarted = false;
                break;

            case BALLISTIC_ARTILLERY:
                m_pPointMass->Init(ArtilleryMass, m_pos, ArtilleryForce, m_pSceneObject);
                m_fireballStarted = false;
                break;

            case BALLISTIC_FIREBALL:
                m_pPointMass->Init(FireballMass, m_pos, FireballForce, m_pSceneObject);
                m_fireballStarted = true;
                break;
            }      
        } 

        if ((m_ballisticType == BALLISTIC_FIREBALL) && m_fireballStarted) {
            m_pPointMass->AddForce(FireballAccel);
        }
        
        m_physicsSystem.Update(DeltaTime);
    }


    virtual bool OnKeyboard(int key, int action)
    {
        bool ret = false;

        if (action == GLFW_PRESS) {
            switch (key) {
            case GLFW_KEY_SPACE:
                m_pPointMass->Init(1.0f, m_pSceneObject->GetGLMPos(), PistolForce, m_pSceneObject);
                ret = true;
                break;

            case GLFW_KEY_1:
                m_ballisticType = BALLISTIC_PISTOL;
                ret = true;
                break;

            case GLFW_KEY_2:
                m_ballisticType = BALLISTIC_ARTILLERY;
                ret = true;
                break;

            case GLFW_KEY_3:
                m_ballisticType = BALLISTIC_FIREBALL;
                ret = true;
                break;

            default:
                ret = BaseGLApp::OnKeyboard(key, action);
            }
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

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

private:

    Scene* m_pScene = NULL;
    DirectionalLight m_dirLight;
    Physics::System m_physicsSystem;
    SceneObject* m_pSceneObject = NULL;
    Physics::PointMass* m_pPointMass = NULL;

    glm::vec3 m_pos = glm::vec3(-20.0f, 1.5f, 40.0f);
    BallisticType m_ballisticType = BALLISTIC_PISTOL;
    bool m_fireballStarted = false;
};



int main(int argc, char* arg[])
{
    ParticleDynamics demo;
    demo.Start();
}
