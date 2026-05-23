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

    Physics Tutorial 03 - Rigid Body Dynamics
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

bool reset = false;

glm::vec3 ForcePoint(0.0f, 0.1f, 0.0f);
glm::vec3 CenterOfMass = glm::vec3(0.0f, 0.0f, 0.0f);

static void PhysicsUpdateListener(const void* pObject, 
                                  const glm::vec3& Pos, 
                                  const glm::quat& Orientation)
{    
    SceneObject* pSceneObject = (SceneObject*)pObject;

   // GLM_PRINT_VEC3("", Pos);

    if (Pos.y <= 0.0) {
        pSceneObject->SetPosition(0.0f, 3.0f, 0.0);
        reset = true;
    } else {
    	pSceneObject->SetPosition(Pos);
	    pSceneObject->SetQuaternion(Orientation);		
    } 
}


class ParticleDynamics : public BaseGLApp {
public:
    ParticleDynamics() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Physics Tutorial 03 - Rigid Body Dynamics")
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
  
        //Model* pModel = m_pRenderingSystem->LoadModel("../Content/box.obj");
        Model* pModel = m_pRenderingSystem->LoadModel("../Content/Sketchfab/futuristic-cyberpunk-axe/source/model2.glb");
        
       // m_pScene->GetConfig()->ControlSkybox(true);
       // m_pScene->LoadSkybox("../Content/textures/143_hdrmaps_com_free_10K_small.jpg");

        m_pSceneObject = m_pScene->CreateSceneObject(pModel);
        m_pSceneObject->SetPosition(0.0f, 3.0f, 0.0);
        m_pScene->AddToRenderList(m_pSceneObject);

        m_pScene->SetCamera(Vector3f(-13.0f, 12.0f, -20.0f), Vector3f(0.85f, -0.2f, 0.5f));

        m_physicsSystem.Init(100, 100, PhysicsUpdateListener, glm::vec3(0.0f));

        m_pRigidBody = m_physicsSystem.AllocRigidBody();
        glm::vec3 ForceVec(glm::vec3(800.0f, 800.0f, 0.0f));
        m_pRigidBody->Init(1.0f, CenterOfMass, m_pSceneObject->GetGLMPos(), ForceVec, ForcePoint, GLM_DEFAULT_QUAT, m_pSceneObject);
        float Width = 1.0f;
        float Height = 2.25f;
        float Depth = 0.3f;
        m_pRigidBody->SetShapeBox(Width, Height, Depth);

        m_pRenderingSystem->Execute();
    }


    void OnFrameChild(double DeltaTime)
    {  
        m_pRigidBody->AddForce(glm::vec3(0.0f, -9.81f, 0.0f));

        m_physicsSystem.Update(DeltaTime);
      
  	    if (reset) {
            reset = false;
            m_pRigidBody->Init(1.0f, CenterOfMass, m_pSceneObject->GetGLMPos(), 
                glm::vec3(RandomFloatRange(700.0f, 900.f),
                          RandomFloatRange(700.0f, 900.0f), 
                          RandomFloatRange(-20.0f, 20.0f)), ForcePoint, GLM_DEFAULT_QUAT, m_pSceneObject);            
        }
    }


    virtual bool OnKeyboard(int key, int action)
    {
        bool ret = false;

        if ((key == GLFW_KEY_SPACE) && (action == GLFW_PRESS)) {
            //m_pPointMass2->Init(1.0f, glm::vec3(0.0f, 1.0f, 5.0), glm::vec3(0.0f, 0.0f, 3.0f), m_pSphere2);
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
    SceneObject* m_pSceneObject = NULL;
    Physics::System m_physicsSystem;
    Physics::RigidBody* m_pRigidBody = NULL;
};



int main(int argc, char* arg[])
{
    ParticleDynamics demo;
    demo.Start();
}
