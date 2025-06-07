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

    DemoLITION - Carbonara
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imGuIZMOquat.h"

#include "demolition.h"
#include "demolition_base_gl_app.h"
#include "ogldev_physics.h"

//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/glm.hpp>
//#include <glm/ext.hpp>

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

#define NUM_PSOs 1000

struct PhysicsSceneObject {
    SceneObject* pSceneObject = NULL;
    OgldevPhysics::Particle* pParticle = NULL;

    void InitPosition(const Vector3f& Pos)
    {
        if (!pSceneObject) {
            printf("pSceneObject is not initialized\n");
            exit(1);
        }

        pSceneObject->SetPosition(Pos);

        if (pParticle) {
            pParticle->SetPosition(Pos);
        }
    }
};


class Carbonara : public BaseGLApp {
public:
    Carbonara() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Carbonara")
    {
      //  m_dirLight.WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_dirLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_dirLight.DiffuseIntensity = 0.9f;
        m_dirLight.AmbientIntensity = 0.1f;

        m_pointLight.WorldPosition = Vector3f(-10.0f, 10.0f, 0.0f);
     //  m_pointLight.WorldPosition = Vector3f(1.0f, 0.0f, -1.0f);
        m_pointLight.DiffuseIntensity = 0.2f;
        m_pointLight.AmbientIntensity = 0.1f;

        m_spotLight.WorldPosition = Vector3f(0.0f, 10.0f, 0.0f);
        m_spotLight.WorldDirection = Vector3f(0.0f, -1.0f, 0.0f);
        m_spotLight.Up = Vector3f(0.0f, 0.0f, -1.0f);
        m_spotLight.Cutoff = 30.0f;
        m_spotLight.DiffuseIntensity = 0.2f;
        m_spotLight.AmbientIntensity = 0.1f;
    }

    ~Carbonara() {}

    virtual void InitChild()
    {
        printf("-------------------------------------------\n");
        printf("Warning! Child didn't implement InitChild()\n");
        printf("-------------------------------------------\n");
        exit(0);
    }

    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();
        
        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
        
      //  m_pScene->SetCameraSpeed(0.1f);

        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = true;
        m_pScene->GetDirLights().push_back(m_dirLight);        
        //m_pScene->GetPointLights().push_back(m_pointLight);
      //  m_pScene->GetSpotLights().push_back(m_spotLight);

        m_pRenderingSystem->SetScene(m_pScene);

        InitChild();

        m_pRenderingSystem->Execute();
    }


    void InitSpringDemo()
    {
        m_pScene->SetCamera(Vector3f(0.0f, 0.5f, -5.0f), Vector3f(0.0, -0.1f, 1.0f));

        Model* pModel = m_pRenderingSystem->LoadModel("../Content/box.obj");

        PhysicsSceneObject Box1;
        Box1.pSceneObject = m_pScene->CreateSceneObject(pModel);
        Box1.pSceneObject->SetScale(0.1f);
        Box1.pParticle = m_physicsSystem.AllocParticle();
        m_sceneObjects.push_back(Box1);
        m_pScene->AddToRenderList(Box1.pSceneObject);
        Box1.pParticle->SetPosition(Vector3f(-1.0f, 0.0f, 0.0f));
        Box1.pParticle->SetMass(2.0f);

        PhysicsSceneObject Box2;
        Box2.pSceneObject = m_pScene->CreateSceneObject(pModel);
        Box2.pSceneObject->SetScale(0.1f);
        Box2.pParticle = m_physicsSystem.AllocParticle();
        m_sceneObjects.push_back(Box2);
        m_pScene->AddToRenderList(Box2.pSceneObject);
        Box2.pParticle->SetPosition(Vector3f(1.0f, 0.0f, 0.0f));
        Box2.pParticle->SetMass(2.0f);

    //    Box.pParticle->SetVelocity(Vector3f(1.0f, 1.0f, 0.0f));
        m_springForceGenerator.Init(Box1.pParticle, 1.0f, 0.99f);

        m_physicsSystem.GetRegistry().Add(Box1.pParticle, &m_springForceGenerator);

//        m_physicsSystem.GetRegistry().Add(CObject.pParticle, &m_gravityForceGenerator);
  //      m_physicsSystem.GetRegistry().Add(CObject.pParticle, &m_dragForceGenerator);

    }


    void InitBuoyancyDemo()
    {
        m_pScene->SetCamera(Vector3f(0.0f, 10.5f, -5.0f), Vector3f(0.0, -0.1f, 1.0f));

        PhysicsSceneObject Water = LoadAndAddModel("../Content/ground.obj", true);
        Water.pParticle->SetPosition(0.0f, 10.0f, 0.0f);
        PhysicsSceneObject Box = LoadAndAddModel("../Content/box.obj", true, 0.1f);
        Box.pParticle->SetMass(0.0001f);
        Box.pParticle->SetPosition(0.0f, 9.5f, 0.0f);

        m_buoyancyForceGenerator.Init(5.0f, 1.0f, 10.0f, 0.1f);
        m_fakeSpringForceGenerator.Init(&Water.pParticle->GetPosition(), 0.000001f, 0.01f);
        m_physicsSystem.GetRegistry().Add(Box.pParticle, &m_fakeSpringForceGenerator);
    }


    void OnFrame(long long DeltaTimeMillis)
    {
        OnFrameChild(DeltaTimeMillis);

      //  if (m_pScene->GetDirLights().size() > 0) {
      //      m_pScene->GetDirLights()[0].WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
      //  }

        m_count += 0.01f;

        if (m_pScene->GetPickedSceneObject()) {
            m_pickedObject = m_pScene->GetPickedSceneObject();
            m_pickedObject->SetColorMod(2.0f, 1.0f, 1.0f);
        } else {
            if (m_pickedObject) {
                m_pickedObject->SetColorMod(1.0f, 1.0f, 1.0f);
                m_pickedObject = NULL;
            }
        }

        m_physicsSystem.Update(DeltaTimeMillis);

        UpdateParticlePositions();
        
    //    m_pSceneObject->ResetRotations();
     //   m_pSceneObject->PushRotation(Vector3f(-90.0f, 0.0f, 0.0f));
        //m_pSceneObject->PushRotation(Vector3f(0.0f, 90.0f, 0.0f));

     //   m_pScene->GetPointLights()[0].WorldPosition.x = sinf(m_count);
      //  m_pScene->GetPointLights()[0].WorldPosition.z = cosf(m_count);
    }


    void OnFrameEnd()
    {
        if (m_showGui) {
            OnFrameGUI();
            ApplyGUIConfig();
        }
    }


    void UpdateParticlePositions()
    {
        for (std::list<PhysicsSceneObject>::iterator it = m_sceneObjects.begin(); it != m_sceneObjects.end(); it++) {
            if (it->pParticle) {
                const Vector3f& NewPos = it->pParticle->GetPosition();
                it->pSceneObject->SetPosition(NewPos);
            }
        }
    }


    PhysicsSceneObject LoadAndAddModel(const char* pFilename, bool AllocParticle, float Scale = 1.0f)
    {
        Model* pModel = m_pRenderingSystem->LoadModel(pFilename);
        PhysicsSceneObject CObject = AddPhysicsSceneObject(pModel, AllocParticle, Scale);
  //      m_physicsSystem.GetRegistry().Add(CObject.pParticle, &m_gravityForceGenerator);
   //     m_physicsSystem.GetRegistry().Add(CObject.pParticle, &m_dragForceGenerator);
        return CObject;
    }


    PhysicsSceneObject AddPhysicsSceneObject(Model* pModel, bool AllocParticle, float Scale = 1.0f)
    {
        PhysicsSceneObject PSObject;
        PSObject.pSceneObject = m_pScene->CreateSceneObject(pModel);
        PSObject.pSceneObject->SetScale(Scale);
        if (AllocParticle) {
            PSObject.pParticle = m_physicsSystem.AllocParticle();
        }        
        m_sceneObjects.push_back(PSObject);
        m_pScene->AddToRenderList(PSObject.pSceneObject);
        return PSObject;
    }


    void OnFrameGUI()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Scene* pScene = m_pRenderingSystem->GetScene();

        bool my_tool_active = false;

        ImGui::Begin("Test", &my_tool_active, ImGuiWindowFlags_MenuBar);                          // Create a window called "Hello, world!" and append into it.

        GUIMenu();

        GUICamera(pScene);

        GUIScene(pScene);

        // ImGui::SliderFloat("Max height", &this->m_maxHeight, 0.0f, 1000.0f);
        // ImGui::SliderFloat("Terrain roughness", &this->m_roughness, 0.0f, 5.0f);

        // ImGui::SliderFloat("Height0", &Height0, 0.0f, 64.0f);
        //  ImGui::SliderFloat("Height1", &Height1, 64.0f, 128.0f);
        //   ImGui::SliderFloat("Height2", &Height2, 128.0f, 192.0f);
        //  ImGui::SliderFloat("Height3", &Height3, 192.0f, 256.0f);

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


    void GUIMenu()
    {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
                if (ImGui::MenuItem("Close", "Ctrl+W")) {  }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }


    void GUICamera(Scene* pScene)
    {
        if (ImGui::TreeNode("Camera")) {
            const GLMCameraFirstPerson* pCamera = pScene->GetCurrentCamera();
            const Vector3f& Pos = pCamera->GetPos();
            const Vector3f& Target = pCamera->GetTarget();
            const Vector3f& Up = pCamera->GetUp();
            ImGui::Text("Position %.3f,%.3f,%.3f", Pos.x, Pos.y, Pos.z);
            ImGui::Text("Target %.3f,%.3f,%.3f", Target.x, Target.y, Target.z);
            ImGui::Text("Up %.3f,%.3f,%.3f", Up.x, Up.y, Up.z);
            ImGui::TreePop();
        }
    }


    void GUIScene(Scene* pScene)
    {
        std::list<SceneObject*> SceneObjectsList = pScene->GetSceneObjectsList();

        if (ImGui::TreeNode("Scene")) {
            ImGui::CheckboxFlags("Enable Shadow Mapping", &m_enableShadowMapping, 1);

            GUILighting(pScene);

            for (std::list<SceneObject*>::const_iterator it = SceneObjectsList.begin(); it != SceneObjectsList.end(); it++) {
                if (ImGui::TreeNode((*it)->GetName().c_str())) {
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
    }


    void GUILighting(Scene* pScene)
    {
        GUIDirLighting(pScene);

        GUIPointLighting(pScene);
    }


    void GUIDirLighting(Scene* pScene)
    {
        if (pScene->GetDirLights().size() > 0) {
            ImGui::Text("Directional Light");
            DirectionalLight& DirLight = pScene->GetDirLights()[0];
            vec3 Dir(DirLight.WorldDirection.x, DirLight.WorldDirection.y, -DirLight.WorldDirection.z);
            ImGui::gizmo3D("##Dir1", Dir /*, size,  mode */);
            DirLight.WorldDirection = Vector3f(Dir.x, Dir.y, -Dir.z);
            Vector3f DiffuseColor = DirLight.Color;
            ImGui::ColorEdit4("Diffuse Color", DiffuseColor.data());
            DirLight.Color = DiffuseColor;
            //    Vector3f AmbientColor = DirLight.;
             //   ImGui::ColorEdit4("Color", Color.data());
             //   DirLight.Color = Color;

        }
    }


    void GUIPointLighting(Scene* pScene)
    {
        if (pScene->GetPointLights().size() > 0) {
            ImGui::Text("Point Light");
            PointLight& Light = pScene->GetPointLights()[0];
            vec3 Pos(Light.WorldPosition.x, Light.WorldPosition.y, Light.WorldPosition.z);
            ImGui::SliderFloat3("Position", &Pos.x, -1000.0f, 1000.0f);
            Light.WorldPosition = Vector3f(Pos.x, Pos.y, Pos.z);
            Vector3f Color = Light.Color;
            ImGui::ColorEdit4("Color", Color.data());
            Light.Color = Color;
        }
    }


    void ApplyGUIConfig()
    {
        m_pScene->GetConfig()->ControlShadowMapping(m_enableShadowMapping);
    }


    bool OnKeyboard(int key, int action)
    {
        bool HandledByMe = false;

        switch (key) {
        case GLFW_KEY_SPACE:
            if (action == GLFW_PRESS) {
                m_showGui = !m_showGui;
            }
            HandledByMe = true;
            break;

        default:
            HandledByMe = BaseGLApp::OnKeyboard(key, action);
        }

        return HandledByMe;
    }


    bool OnMouseMove(int x, int y)
    {
        return !m_leftMousePressed;
    }


    bool OnMouseButton(int Button, int Action, int Mode, int x, int y)
    {
        bool HandledByMe = true;

        switch (Button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            m_leftMousePressed = (Action == GLFW_PRESS);
            break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            m_midMousePressed = (Action == GLFW_PRESS);
            m_pScene->GetConfig()->ControlPicking(m_midMousePressed);
            break;

        default:
            HandledByMe = false;
        }

        return false;
    }

protected:

    virtual void OnFrameChild(long long DeltaTimeMillis) {}

    Scene* m_pScene = NULL;
    std::list<PhysicsSceneObject> m_sceneObjects;
    OgldevPhysics::PhysicsSystem m_physicsSystem;

private:

    float m_count = 0.0f;

    DirectionalLight m_dirLight;
    PointLight m_pointLight;
    SpotLight m_spotLight;
    bool m_leftMousePressed = false;
    bool m_midMousePressed = false;
    SceneObject* m_pickedObject = NULL;
    bool m_showGui = false;
    int m_enableShadowMapping = 1;

    OgldevPhysics::GravityForceGenerator m_gravityForceGenerator;
    OgldevPhysics::DragForceGenerator m_dragForceGenerator;
    OgldevPhysics::SpringForceGenerator m_springForceGenerator;
    OgldevPhysics::BuoyancyForceGenerator m_buoyancyForceGenerator;
    OgldevPhysics::FakeSpringForceGenerator m_fakeSpringForceGenerator;
};



class BallisticsDemo : public Carbonara {

public:

    BallisticsDemo()
    {
        m_physicsSystem.Init(NUM_PSOs, 1, 1);
    }

    void InitChild()
    {
        m_pScene->SetCamera(Vector3f(0.0f, 0.5f, -5.0f), Vector3f(0.0, -0.1f, 1.0f));

        PhysicsSceneObject Ground = LoadAndAddModel("../Content/ground.obj", true);
        PhysicsSceneObject Box = LoadAndAddModel("../Content/box.obj", true, 0.1f);

        Box.pParticle->SetPosition(Vector3f(-1.0f, 0.0f, 0.0f));
        Box.pParticle->SetMass(2.0f);
        Box.pParticle->SetVelocity(Vector3f(1.0f, 1.0f, 0.0f));
        //   Box.pParticle->SetDamping(0.99f);
    }
};


class FireworksDemo : public Carbonara {

public:

    FireworksDemo()  
    {
        m_physicsSystem.Init(NUM_PSOs, 1, 1);
    }

    void InitChild()
    {
        m_pScene->SetCamera(Vector3f(0.0f, 0.0f, -50.0f), Vector3f(0.0, 0.1f, 1.0f));

        int NumFireworks = 50;

        Model* pModel = m_pRenderingSystem->LoadModel("../Content/box.obj");

        for (int i = 0; i < NumFireworks; i++) {
            PhysicsSceneObject CObject;
            CObject.pSceneObject = m_pScene->CreateSceneObject(pModel);
            CObject.pSceneObject->SetScale(0.1f);
            CObject.pParticle = m_physicsSystem.AllocFirework();
            m_sceneObjects.push_back(CObject);
            m_pScene->AddToRenderList(CObject.pSceneObject);
        }
    }
};


class AnimationDemo : public Carbonara {

public:

    AnimationDemo() {}

    void InitChild()
    {
        m_pScene->SetCamera(Vector3f(0.0f, 0.25f, -0.4f), Vector3f(0.0, -0.4f, 1.0f));
        PhysicsSceneObject CObject = LoadAndAddModel("../Content/Jump/Jump.dae", false, 1.0f);
        CObject.pSceneObject->SetRotation(0.0f, 180.0f, 0.0f);
        m_pScene->GetConfig()->ControlShadowMapping(false);
    }
};


static void DirToRotation(const Vector3f& Dir, SceneObject& o)
{
    Vector3f DirN = Dir;
    DirN.Normalize();

    glm::vec3 Start(1.0f, 0.0f, 0.0f);
    glm::vec3 Dest(Dir.x, Dir.y, Dir.z);

    glm::quat q = RotationBetweenVectors(Start, Dest);
    o.SetQuaternion(q);
}


class BridgeDemo : public Carbonara {

public:

#define NUM_SPHERES 12
#define NUM_CABLES (NUM_SPHERES-2)
#define NUM_PLANKS (NUM_SPHERES / 2)

#define PLANK_LENGTH 2.0f

#define BASE_MASS 1
#define EXTRA_MASS 10

    BridgeDemo() 
    {
        m_physicsSystem.Init(NUM_PSOs, NUM_SPHERES * 10, 0);
    }

    void InitChild()
    {
       // glDisable(GL_CULL_FACE);
        m_pScene->SetCamera(Vector3f(0.0f, 4.5f, -14.0f), Vector3f(0.0, -0.25f, 1.0f));

        m_pRod = m_pRenderingSystem->LoadModel("../Content/demolition/rod.obj");
        int SphereTexture = m_pRenderingSystem->LoadTexture2D("../Content/textures/brickwall.jpg");
        m_pRod->SetColorTexture(SphereTexture);

        m_pSphere = m_pRenderingSystem->LoadModel("../Content/demolition/sphere8.obj");
        m_pSphere->SetColorTexture(SphereTexture);

        m_PSOs.resize(NUM_SPHERES);
        m_particles.resize(NUM_SPHERES);
        m_supports.resize(NUM_SPHERES);
        m_vLinks.resize(NUM_SPHERES);
        m_hLinks.resize(NUM_CABLES);
        m_cables.resize(NUM_CABLES);
        m_planks.resize(NUM_PLANKS);
        m_rods.resize(NUM_PLANKS);

        InitPlankEnds();

        InitPlanks();
        
        InitVLinks();

        InitHLinks();

        InitBall();

        m_groundContactGenerator.Init(&m_particles);

        m_physicsSystem.AddContactGenerator(&m_groundContactGenerator);
    }


    bool OnKeyboard(int key, int action)
    {
        bool HandledByMe = false;

        switch (key) {
        case GLFW_KEY_U:
            m_ballBasePos.z += 0.1f;
            HandledByMe = true;
            break;

        case GLFW_KEY_M:
            m_ballBasePos.z -= 0.1f;
            HandledByMe = true;
            break;

        case GLFW_KEY_H:
            m_ballBasePos.x -= 0.1f;
            HandledByMe = true;
            break;

        case GLFW_KEY_K:
            m_ballBasePos.x += 0.1f;
            HandledByMe = true;
            break;

        default:
            HandledByMe = Carbonara::OnKeyboard(key, action);
        }

        m_ballBasePos.x = CLAMP(m_ballBasePos.x, 0.0f, 5.0f);
        m_ballBasePos.z = CLAMP(m_ballBasePos.z, -1.0f, 1.0f);
        return HandledByMe;
    }


protected:

    virtual void OnFrameChild(long long DeltaTimeMillis)
    {
        // Links from above
        for (int i = 0; i < NUM_SPHERES; i++) {
            Vector3f Dir = m_supports[i].m_anchor - m_PSOs[i].pSceneObject->GetPosition();
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[i].pSceneObject->GetPosition() + Dir / 2.0f;
            m_vLinks[i]->SetPosition(Pos);
            m_vLinks[i]->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *m_vLinks[i]);
        }

        for (int i = 0; i < NUM_CABLES; i++) {
            Vector3f Dir = (m_PSOs[i + 2].pSceneObject->GetPosition() - m_PSOs[i].pSceneObject->GetPosition());
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[i].pSceneObject->GetPosition() + Dir / 2.0f;
            m_hLinks[i]->SetPosition(Pos);
            m_hLinks[i]->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *m_hLinks[i]);
        }

        for (int i = 0; i < NUM_PLANKS; i++) {
            int BaseIndex = i * 2;
            Vector3f Dir = (m_PSOs[BaseIndex + 1].pSceneObject->GetPosition() - m_PSOs[BaseIndex].pSceneObject->GetPosition());
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[BaseIndex].pSceneObject->GetPosition() + Dir / 2.0f;
            m_planks[i]->SetPosition(Pos);
            m_planks[i]->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *m_planks[i]);
        }

        UpdateBallPosition();
    }

    
    void UpdateBallPosition()
    {
        int x = int(m_ballBasePos.x);

        float xp = fmod(m_ballBasePos.x, 1.0f);

        if (x < 0) {
            x = 0;
            xp = 0;
        } 
        
        if (x >= 5) {
            x = 5;
            xp = 0;
        }

        int z = int(m_ballBasePos.z);
        float zp = fmod(m_ballBasePos.z, 1.0f);

        if (z < 0) {
            z = 0;
            zp = 0.0f;
        }

        if (z >= 1) {
            z = 1;
            zp = 0;
        }


        // Calculate where to draw the mass
        m_ballDisplayPos.SetZero();

        // Add the proportion to the correct masses
        m_particles[x * 2 + z]->SetMass(BASE_MASS + EXTRA_MASS * (1 - xp) * (1 - zp));

        m_ballDisplayPos += (m_particles[x * 2 + z]->GetPosition() * (1 - xp) * (1 - zp));

        if (xp > 0) {
            m_particles[x * 2 + z + 2]->SetMass(BASE_MASS + EXTRA_MASS * xp * (1 - zp));
            m_ballDisplayPos += (m_particles[x * 2 + z + 2]->GetPosition() * xp * (1 - zp));

            if (zp > 0) {
                m_particles[x * 2 + z + 3]->SetMass(BASE_MASS + EXTRA_MASS * xp * zp);
                m_ballDisplayPos += (m_particles[x * 2 + z + 3]->GetPosition() * xp * zp);
            }
        }

        if (zp > 0) {
            m_particles[x * 2 + z + 1]->SetMass(BASE_MASS + EXTRA_MASS * (1 - xp) * zp);
            m_ballDisplayPos += (m_particles[x * 2 + z + 1]->GetPosition() * (1 - xp) * zp);
        }

        m_ballDisplayPos.y += 0.5f;

        m_ballPSObject.InitPosition(m_ballDisplayPos);
    }

private:

    void InitPlankEnds()
    {
        // Base spheres
        for (int i = 0; i < NUM_SPHERES; i++) {
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pSphere, true, 0.05f);
            Vector3f Pos((i / 2.0f) * 2.0f - 5.0f, 1.0f, (i % 2) * 2.0f - 1.0f);
            PSObject.InitPosition(Pos);
            PSObject.pParticle->SetMass(1.0f);
            PSObject.pParticle->SetDamping(0.9f);
            PSObject.pParticle->SetAcceleration(OgldevPhysics::GRAVITY);
            m_PSOs[i] = PSObject;
            m_particles[i] = PSObject.pParticle;
        }
    }

    void InitPlanks()
    {
        // Rods that connect each opposing spheres
        for (int i = 0; i < NUM_PLANKS; i++) {
            int BaseIndex = i * 2;
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pRod, false);
            Vector3f Dir = (m_PSOs[BaseIndex + 1].pSceneObject->GetPosition() - m_PSOs[BaseIndex].pSceneObject->GetPosition());
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[BaseIndex].pSceneObject->GetPosition() + Dir / 2.0f;
            m_planks[i] = PSObject.pSceneObject;
            PSObject.pSceneObject->SetPosition(Pos);
            PSObject.pSceneObject->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *PSObject.pSceneObject);
            m_rods[i].m_pParticles[0] = m_PSOs[BaseIndex].pParticle;
            m_rods[i].m_pParticles[1] = m_PSOs[BaseIndex + 1].pParticle;
            m_rods[i].m_len = PLANK_LENGTH;
            m_physicsSystem.AddContactGenerator(&m_rods[i]);
        }
    }

    void InitVLinks()
    {
        // Links from above
        for (int i = 0; i < NUM_SPHERES; i++) {
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pRod, false);
            Vector3f AnchorPos((i / 2.0f) * 2.2f - 5.5f, 6, (i % 2) * 1.6f - 0.8f);
            Vector3f Dir = AnchorPos - m_PSOs[i].pSceneObject->GetPosition();
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[i].pSceneObject->GetPosition() + Dir / 2.0f;
            PSObject.pSceneObject->SetPosition(Pos);
            PSObject.pSceneObject->SetScale(RodLen / 2.0f);
            m_vLinks[i] = PSObject.pSceneObject;
            DirToRotation(Dir, *PSObject.pSceneObject);
            m_supports[i].m_pParticle = m_PSOs[i].pParticle;
            m_supports[i].m_anchor = AnchorPos;
            float MaxLength = 0.0f;
            
            if (i < 6) {
                MaxLength = (i / 2.0f) * 0.5f + 3.0f;
            } else {
                MaxLength = 5.5f - (i / 2.0f) * 0.5f;
            }

            m_supports[i].m_maxLength = MaxLength;
            m_supports[i].m_restitution = 0.5f;
            m_physicsSystem.AddContactGenerator(&m_supports[i]);
        }
    }

    void InitHLinks()
    {
        // Links between consecutive spheres
        for (int i = 0; i < NUM_CABLES ; i++) {
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pRod, false);
            m_cables[i].m_pParticles[0] = m_PSOs[i].pParticle;
            m_cables[i].m_pParticles[1] = m_PSOs[i+2].pParticle;
            m_cables[i].m_maxLength = 1.9f;
            m_cables[i].m_restituion = 0.3f;
            m_physicsSystem.AddContactGenerator(&m_cables[i]);
            Vector3f Dir = (m_PSOs[i + 2].pSceneObject->GetPosition() - m_PSOs[i].pSceneObject->GetPosition());
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[i].pSceneObject->GetPosition() + Dir / 2.0f;
            m_hLinks[i] = PSObject.pSceneObject;
            PSObject.pSceneObject->SetPosition(Pos);
            PSObject.pSceneObject->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *PSObject.pSceneObject);
        }
    }

    void InitBall()
    {
        int BallTexture = m_pRenderingSystem->LoadTexture2D("../Content/textures/gutsy.png");
        m_pBall = m_pRenderingSystem->LoadModel("../Content/demolition/sphere8.obj");
        m_pBall->SetColorTexture(BallTexture);
        m_ballPSObject = AddPhysicsSceneObject(m_pBall, true, 0.5f);
    }

    std::vector<PhysicsSceneObject> m_PSOs;
    std::vector<SceneObject*> m_vLinks;
    std::vector<SceneObject*> m_hLinks;
    std::vector<SceneObject*> m_planks;
    std::vector<OgldevPhysics::Particle*> m_particles;
    std::vector<OgldevPhysics::ParticleCable> m_cables;
    std::vector<OgldevPhysics::ParticleCableConstraint> m_supports;
    std::vector<OgldevPhysics::ParticleRod> m_rods;
    OgldevPhysics::GroundContacts m_groundContactGenerator;
    Model* m_pSphere = NULL;
    Model* m_pBall = NULL;
    Model* m_pRod = NULL;
    PhysicsSceneObject m_ballPSObject;
    Vector3f m_ballBasePos;
    Vector3f m_ballDisplayPos;
};


class AssetLoadDemo : public Carbonara {

public:

    AssetLoadDemo()
    {

    }

    void InitChild()
    {
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = true;
        m_pScene->GetConfig()->ControlShadowMapping(false);
        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

     //   m_pScene->SetCamera(Vector3f(-490.0f, 270.0f, 570.0f), Vector3f(1.0f, 0.05f, 0.4f));
      //  Model* pModel = m_pRenderingSystem->LoadModel("C:/Users/emeir/Downloads/Bistro_v5_2/Bistro_v5_2/BistroExterior.fbx");
      //   Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/McGuire/bistro/Exterior/exterior.obj");

       Model* pModel = m_pRenderingSystem->LoadModel("../Content/DamagedHelmet/glTF/DamagedHelmet.gltf");
       // Model* pModel = m_pRenderingSystem->LoadModel("../Content/crytek_sponza/sponza.obj");
        
        SceneObject* pSceneObject = m_pScene->CreateSceneObject(pModel);
     //   pSceneObject->SetPosition(0.0f, 0.0f, 10.0f);
        m_pScene->SetCamera(Vector3f(0.0f, 1.0f, -2.5f), Vector3f(0.0f, 0.0f, 1.0f));

        m_pScene->AddToRenderList(pSceneObject);
    }
};


class PBRDemo : public Carbonara {

public:

    PBRDemo()
    {
        float metalRough = 0.43f;

        // Gold
        m_color[0] = Vector3f(1, 0.71f, 0.29f);
        // Copper
        m_color[1] = Vector3f(0.95f, 0.64f, 0.54f);
        // Aluminum
        m_color[2] = Vector3f(0.91f, 0.92f, 0.92f);
        // Titanium
        m_color[3] = Vector3f(0.542f, 0.497f, 0.449f);
        // Silver
        m_color[4] = Vector3f(0.95f, 0.93f, 0.88f);

    }

    void InitChild()
    {
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = true;
        m_pScene->GetConfig()->ControlShadowMapping(false);
        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

        m_pModel = m_pRenderingSystem->LoadModel("../Content/DamagedHelmet/glTF/DamagedHelmet.gltf");
      //  m_pModel = m_pRenderingSystem->LoadModel("../Content/dragon.obj");

        m_pTexAO = new Texture(GL_TEXTURE_2D, "../Content/DamagedHelmet/glTF/Default_AO.jpg");
        m_pTexAO->Load();
        m_pTexEmissive = new Texture(GL_TEXTURE_2D, "../Content/DamagedHelmet/glTF/Default_emissive.jpg");
        m_pTexEmissive->Load();
        m_pTexAlbedo = new Texture(GL_TEXTURE_2D, "../Content/DamagedHelmet/glTF/Default_albedo.jpg");
        m_pTexAlbedo->Load();
        m_pTexMeR = new Texture(GL_TEXTURE_2D, "../Content/DamagedHelmet/glTF/Default_metalRoughness.jpg");
        m_pTexMeR->Load();
        m_pTexNormal = new Texture(GL_TEXTURE_2D, "../Content/DamagedHelmet/glTF/Default_normal.jpg");
        m_pTexNormal->Load();

        m_pModel->SetPBR(true);
        m_pModel->GetPBRMaterial().pAO = m_pTexAO;
        m_pModel->GetPBRMaterial().pEmissive = m_pTexEmissive;
        m_pModel->GetPBRMaterial().pAlbedo = m_pTexAlbedo;
        m_pModel->GetPBRMaterial().pMetallic = m_pTexMeR;
        m_pModel->GetPBRMaterial().pNormalMap = m_pTexNormal;

        SceneObject* pSceneObject = m_pScene->CreateSceneObject(m_pModel);
        m_pScene->AddToRenderList(pSceneObject);

        m_pScene->SetCamera(Vector3f(0.0f, 1.0f, -4.0f), Vector3f(0.0f, 0.0f, 1.0f));        
    }

    void OnFrameChild(long long DeltaTimeMillis)
    {
        m_pModel->GetPBRMaterial().Roughness = 0.43f;
        m_pModel->GetPBRMaterial().IsMetal = true;
        m_pModel->GetPBRMaterial().Color = m_color[0];

        float Roughness[] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };

/*        for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_color); i++) {
            m_pModel->SetPosition(m_color[i].Pos );
            m_pModel->GetPBRMaterial().Roughness = Roughness[i];
            m_pModel->GetPBRMaterial().IsMetal = false;
            m_pModel->GetPBRMaterial().Color = Vector3f(0.1f, 0.33f, 0.17f);
            m_phongRenderer.Render(m_pModel);
        }*/

        m_time += DeltaTimeMillis;
    }

private:

    Model* m_pModel = NULL;

    Vector3f m_color[5];
    long long m_time = 0;

    Texture* m_pTexAO = NULL;
    Texture* m_pTexEmissive = NULL;
    Texture* m_pTexAlbedo = NULL;
    Texture* m_pTexMeR = NULL;
    Texture* m_pTexNormal = NULL;

};

class GLTFPBRDemo : public Carbonara {

public:

    GLTFPBRDemo()
    {
        float metalRough = 0.43f;

        // Gold
        m_color[0] = Vector3f(1, 0.71f, 0.29f);
        // Copper
        m_color[1] = Vector3f(0.95f, 0.64f, 0.54f);
        // Aluminum
        m_color[2] = Vector3f(0.91f, 0.92f, 0.92f);
        // Titanium
        m_color[3] = Vector3f(0.542f, 0.497f, 0.449f);
        // Silver
        m_color[4] = Vector3f(0.95f, 0.93f, 0.88f);

    }

    void InitChild()
    {
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = false;
        m_pScene->GetConfig()->ControlShadowMapping(false);
        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

        m_pModel = m_pRenderingSystem->LoadModel("../Content/DamagedHelmet/glTF/DamagedHelmet.gltf");
        //  m_pModel = m_pRenderingSystem->LoadModel("../Content/dragon.obj");

        m_pTexAO = new Texture(GL_TEXTURE_2D, "../Content/DamagedHelmet/glTF/Default_AO.jpg");
        m_pTexAO->Load();

        m_pModel->SetPBR(true);
        m_pModel->GetPBRMaterial().pAO = m_pTexAO;

        int EnvMap = m_pRenderingSystem->LoadCubemapTexture("../Content/textures/piazza_bologni_1k_prefilter.ktx");

        m_pScene->GetConfig()->SetEnvMap(EnvMap);

        m_pSceneObject = m_pScene->CreateSceneObject(m_pModel);
        m_pScene->AddToRenderList(m_pSceneObject);

        m_pScene->SetCamera(Vector3f(0.0f, 1.0f, -4.0f), Vector3f(0.0f, 0.0f, 1.0f));
    }

    void OnFrameChild(long long DeltaTimeMillis)
    {
        m_pModel->GetPBRMaterial().Roughness = 0.43f;
        m_pModel->GetPBRMaterial().IsMetal = true;
        m_pModel->GetPBRMaterial().Color = m_color[0];

        float Roughness[] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };

        m_pSceneObject->RotateBy(0.0f, 0.0f, 0.5f);

        /*        for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_color); i++) {
                    m_pModel->SetPosition(m_color[i].Pos );
                    m_pModel->GetPBRMaterial().Roughness = Roughness[i];
                    m_pModel->GetPBRMaterial().IsMetal = false;
                    m_pModel->GetPBRMaterial().Color = Vector3f(0.1f, 0.33f, 0.17f);
                    m_phongRenderer.Render(m_pModel);
                }*/
    }

private:

    Model* m_pModel = NULL;
    SceneObject* m_pSceneObject = NULL;

    Vector3f m_color[5];
    Texture* m_pTexAO = NULL;
};



class SkyboxDemo : public Carbonara {

public:

    SkyboxDemo()
    {

    }

    void InitChild()
    {
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = false;
        m_pScene->GetConfig()->ControlShadowMapping(false);
        m_pScene->GetConfig()->ControlSkybox(true);
        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

        //   m_pScene->SetCamera(Vector3f(-490.0f, 270.0f, 570.0f), Vector3f(1.0f, 0.05f, 0.4f));

        Model* pModel = m_pRenderingSystem->LoadModel("../Content/DamagedHelmet/glTF/DamagedHelmet.gltf");

        SceneObject* pSceneObject = m_pScene->CreateSceneObject(pModel);
        //   pSceneObject->SetPosition(0.0f, 0.0f, 10.0f);
        m_pScene->SetCamera(Vector3f(0.0f, 1.0f, -2.5f), Vector3f(0.0f, 0.0f, 1.0f));

        m_pScene->LoadSkybox("../Content/textures/ahornsteig_4k.jpg");

        m_pScene->AddToRenderList(pSceneObject);
    }
};


void carbonara()
{
  // BallisticsDemo demo;
   //FireworksDemo demo;
  //  AnimationDemo demo;
  //  BridgeDemo demo;
 //   AssetLoadDemo demo;
  //  PBRDemo demo;
  //  SkyboxDemo demo;
      GLTFPBRDemo demo;

    demo.Start();
}
