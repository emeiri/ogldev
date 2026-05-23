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

    Physics Tutorial 04 - Rigid Body Collision
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


class ParticleDynamics : public BaseGLApp {

public:

    ParticleDynamics() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Physics Tutorial 04 - Rigid Body Collision")
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
        m_pScene->SetCamera(Vector3f(20.0f, 4.0f, 0.0f), Vector3f(0.0f, 0.05f, -1.0f));

        SceneConfig* pConfig = m_pScene->GetConfig();
        pConfig->GetInfiniteGrid().Enabled = true;

        m_pScene->GetDirLights().push_back(m_dirLight);
  
        Model* pModel = m_pRenderingSystem->LoadModel("../PhysicsForGameDev/Tutorial04_RigidBodyCollision/box.obj");
      //  Model* pModel = m_pRenderingSystem->LoadModel("../Content/Sketchfab/futuristic-cyberpunk-axe/source/model2.glb");
        
        pConfig->ControlSkybox(true);
        m_pScene->LoadSkybox("../Content/textures/143_hdrmaps_com_free_10K_small.jpg");

        CreateSceneObjects(pModel);

        m_physicsSystem.Init(100, 100, PhysicsUpdateListener, Physics::GRAVITY);

        CreateRigidBodies();

        m_pRenderingSystem->Execute();
    }


    void CreateRigidBodies()
    {
        {
            m_pRigidBody = m_physicsSystem.AllocRigidBody();
            glm::vec3 ForceVec(glm::vec3(-800.0f, 800.0f, 0.0f));
            m_pRigidBody->Init(1.0f, m_centerOfMass, m_pSceneObject->GetGLMPos(), ForceVec, m_forcePoint, GLM_DEFAULT_QUAT, m_pSceneObject);
            float Width = 2.0f;
            float Height = 6.0f;
            float Depth = 4.0f;
            m_pRigidBody->SetShapeBox(Width, Height, Depth);
            m_pRigidBody->GetLinear().SetBoundingRadius(3.0f);
        }

        {
            m_pRigidBody2 = m_physicsSystem.AllocRigidBody();
            glm::vec3 ForceVec(glm::vec3(0.0f, 800.0f, -800.0f));
            m_pRigidBody2->Init(1.0f, m_centerOfMass, m_pSceneObject2->GetGLMPos(), ForceVec, m_forcePoint2, m_orientation, m_pSceneObject2);
            float Width = 4.0f;
            float Height = 6.0f;
            float Depth = 2.0f;
            m_pRigidBody2->SetShapeBox(Width, Height, Depth);
            m_pRigidBody2->GetLinear().SetBoundingRadius(3.0f);
        }
    }


    void CreateSceneObjects(Model* pModel)
    {
        m_pSceneObject = m_pScene->CreateSceneObject(pModel);
        m_pSceneObject->SetPosition(m_Pos);
        m_pScene->AddToRenderList(m_pSceneObject);

        m_pSceneObject2 = m_pScene->CreateSceneObject(pModel);
        m_pSceneObject2->SetPosition(m_Pos2);
        m_pScene->AddToRenderList(m_pSceneObject2);
    }


    void OnFrameChild(double DeltaTime)
    { 
        if (m_pRigidBody->GetLinear().GetPos().y < -0.01f) {
           // printf("Initializing\n");
            m_pRigidBody->Init(1.0f, m_centerOfMass, m_Pos, 
                glm::vec3(RandomFloatRange(-820.0f, -780.f),
                          RandomFloatRange(780.0f, 820.0f), 
                          RandomFloatRange(-40.0f, 40.0f)), m_forcePoint, GLM_DEFAULT_QUAT, m_pSceneObject);            
        } 
        
        m_physicsSystem.Update(DeltaTime);
    }


    virtual bool OnKeyboard(int key, int action)
    {
        bool ret = false;

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_PRESS)) {
            m_pRigidBody2->Init(1.0f, m_centerOfMass, m_Pos2,
                glm::vec3(RandomFloatRange(-200.0f, 200.f),
                          RandomFloatRange(780.0f, 820.0f),
                          RandomFloatRange(-820.0f, -780.0f)), m_forcePoint2, m_orientation, m_pSceneObject2);
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

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

private:

    Scene* m_pScene = NULL;
    DirectionalLight m_dirLight;
    Physics::System m_physicsSystem;
    SceneObject* m_pSceneObject = NULL;
    SceneObject* m_pSceneObject2 = NULL;
    Physics::RigidBody* m_pRigidBody = NULL;
    Physics::RigidBody* m_pRigidBody2 = NULL;

    glm::vec3 m_forcePoint = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_forcePoint2 = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_centerOfMass = glm::vec3(0.0f, 0.5f, 0.0f);

    glm::vec3 m_Pos = glm::vec3(40.0f, 3.0f, -30.0f);
    glm::vec3 m_Pos2 = glm::vec3(20.0f, 3.0f, -20.0f);

    glm::quat m_orientation = GLM_DEFAULT_QUAT;//  glm::angleAxis(glm::radians(0.0f), glm::vec3(1, 0, 0));
};



int main(int argc, char* arg[])
{
    ParticleDynamics demo;
    demo.Start();
}
