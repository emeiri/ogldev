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

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

#define NUM_PARTICLES 1000

struct PhysicsSceneObject {
    SceneObject* pSceneObject = NULL;
    OgldevPhysics::Particle* pParticle = NULL;
};


class Carbonara : public BaseGLApp {
public:
    Carbonara() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Carbonara")
    {
      //  m_dirLight.WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_dirLight.WorldDirection = Vector3f(0.0f, -1.0f, -1.0f);
        m_dirLight.DiffuseIntensity = 1.0f;

        m_pointLight.WorldPosition = Vector3f(0.25f, 0.25f, 0.0f);
     //  m_pointLight.WorldPosition = Vector3f(1.0f, 0.0f, -1.0f);
        m_pointLight.DiffuseIntensity = 2.0f;
        m_pointLight.AmbientIntensity = 0.1f;

        m_physicsSystem.Init(NUM_PARTICLES, 1, 1);
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
        
        m_pScene->SetCameraSpeed(0.1f);

        m_pScene->GetDirLights().push_back(m_dirLight);
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = true;
      //  m_pScene->GetPointLights().push_back(m_pointLight);

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
        if (m_showGui) {
            OnFrameGUI();
            ApplyGUIConfig();
        }

        if (m_pScene->GetDirLights().size() > 0) {
            m_pScene->GetDirLights()[0].WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        }

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
        PhysicsSceneObject CObject = AddPhysicsSceneObject(pModel, Scale);
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
            const BasicCamera* pCamera = pScene->GetCurrentCamera();
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

        return HandledByMe;
    }

protected:

    Scene* m_pScene = NULL;
    std::list<PhysicsSceneObject> m_sceneObjects;
    OgldevPhysics::PhysicsSystem m_physicsSystem;

private:

    float m_count = 0.0f;

    DirectionalLight m_dirLight;
    PointLight m_pointLight;
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
        m_physicsSystem.Init(NUM_PARTICLES, 1, 1);
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
        m_physicsSystem.Init(NUM_PARTICLES, 1, 1);
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
        m_pScene->SetCamera(Vector3f(0.0f, 0.33f, -0.65f), Vector3f(0.0, -0.3f, 1.0f));
        LoadAndAddModel("../Content/Jump/Jump.dae", false, 0.1f);
    }
};


static void DirToRotation(const Vector3f& Dir, SceneObject& o)
{
    Vector3f DirN = Dir;
    DirN.Normalize();

    float AngleH = ToDegree(atan2(DirN.z, DirN.x));
    float AngleV = ToDegree(asin(DirN.y));

    if (AngleH != 0.0f) {
        o.PushRotation(Vector3f(0.0f, AngleH, 0.0f));
    }

    if (AngleV != 0.0f) {
        o.PushRotation(Vector3f(0.0f, 0.0f, AngleV));
    }
}



class BridgeDemo : public Carbonara {

public:

    BridgeDemo() 
    {
        m_physicsSystem.Init(NUM_PARTICLES, NUM_CABLES, 1);
    }

    void InitChild()
    {
       // glDisable(GL_CULL_FACE);
        m_pScene->SetCamera(Vector3f(0.0f, 4.5f, -14.0f), Vector3f(0.0, -0.25f, 1.0f));

#define NUM_SPHERES 12

        m_pRod = m_pRenderingSystem->LoadModel("../Content/demolition/rod.obj");
        int Texture = m_pRenderingSystem->LoadTexture2D("../Content/textures/brickwall.jpg");
        m_pRod->SetColorTexture(Texture);

        m_pSphere = m_pRenderingSystem->LoadModel("../Content/demolition/sphere8.obj");
        m_pSphere->SetColorTexture(Texture);

        m_spheres.resize(NUM_SPHERES);

        InitPlankEnds();

        InitPlanks();
        
        InitVLinks();

        InitHLinks();
    }

private:

    void InitPlankEnds()
    {
        // Base spheres
        for (int i = 0; i < NUM_SPHERES; i++) {
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pSphere, true, 0.05f);
            PSObject.pSceneObject->SetPosition(Vector3f((i / 2.0f) * 2.0f - 5.0f, 1.0f, (i % 2) * 2.0f - 1.0f));
            PSObject.pParticle->SetDamping(0.9f);
            PSObject.pParticle->SetAcceleration(OgldevPhysics::GRAVITY);
            m_spheres[i] = PSObject.pSceneObject;
        }
    }

    void InitPlanks()
    {
        // Rods that connect each opposing spheres
        for (int i = 0; i < NUM_SPHERES / 2; i++) {
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pRod, false);
            Vector3f Dir = (m_spheres[i * 2 + 1]->GetPosition() - m_spheres[i * 2]->GetPosition());
            float RodLen = Dir.Length();
            Vector3f Pos = m_spheres[i * 2]->GetPosition() + Dir / 2.0f;
            PSObject.pSceneObject->SetPosition(Pos);
            PSObject.pSceneObject->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *PSObject.pSceneObject);
        }
    }

    void InitVLinks()
    {
        // Links from above
        for (int i = 0; i < NUM_SPHERES; i++) {
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pRod, false);
            Vector3f AnchorPos = m_spheres[i]->GetPosition() + Vector3f(0.0f, 2.0f, 0.0f);
            Vector3f Dir = AnchorPos - m_spheres[i]->GetPosition();
            Vector3f Pos = m_spheres[i]->GetPosition() + Dir / 2.0f;
            PSObject.pSceneObject->SetPosition(Pos);
            DirToRotation(Dir, *PSObject.pSceneObject);
        }
    }

    void InitHLinks()
    {
        // Links between consecutive spheres
        for (int i = 0; i < NUM_SPHERES - 2; i++) {
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pRod, false);
            Vector3f Dir = (m_spheres[i + 2]->GetPosition() - m_spheres[i]->GetPosition());
            float RodLen = Dir.Length();
            Vector3f Pos = m_spheres[i]->GetPosition() + Dir / 2.0f;
            PSObject.pSceneObject->SetPosition(Pos);
            DirToRotation(Dir, *PSObject.pSceneObject);
        }
    }

    std::vector<SceneObject*> m_spheres;
    Model* m_pSphere = NULL;
    Model* m_pRod = NULL;
};

void carbonara()
{
  // BallisticsDemo demo;
   //FireworksDemo demo;
   // AnimationDemo demo;
    BridgeDemo demo;

    demo.Start();
}
