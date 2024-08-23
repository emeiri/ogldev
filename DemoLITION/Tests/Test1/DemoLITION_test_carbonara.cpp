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


struct CombinedObject {
    SceneObject* pSceneObject = NULL;
    OgldevPhysics::Particle* pParticle = NULL;
};


class Carbonara : public BaseGLApp {
public:
    Carbonara() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT)
    {
      //  m_dirLight.WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_dirLight.WorldDirection = Vector3f(0.0f, -1.0f, -1.0f);
        m_dirLight.DiffuseIntensity = 1.0f;

        m_pointLight.WorldPosition = Vector3f(0.25f, 0.25f, 0.0f);
     //  m_pointLight.WorldPosition = Vector3f(1.0f, 0.0f, -1.0f);
        m_pointLight.DiffuseIntensity = 2.0f;
        m_pointLight.AmbientIntensity = 0.1f;

        m_physicsSystem.Init();
    }

    ~Carbonara() {}

    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();
        
        InitBallisticDemo();

      //  InitFireworksDemo();
      //  LoadAndAddModel("../Content/Jump/Jump.dae");

        //Grid* pGrid = m_pRenderingSystem->CreateGrid(100, 100);
        //pSceneObject = m_pScene->CreateSceneObject(pGrid);
        //m_pSceneObjects.push_back(pSceneObject);
        //m_pScene->AddToRenderList(pSceneObject);

        m_pScene->SetClearColor(Vector4f(0.0f, 1.0f, 0.0f, 0.0f));
        
        m_pScene->SetCameraSpeed(0.1f);

        m_pScene->GetDirLights().push_back(m_dirLight);
      //  m_pScene->GetPointLights().push_back(m_pointLight);

        m_pRenderingSystem->SetScene(m_pScene);

        m_pRenderingSystem->Execute();
    }


    void InitBallisticDemo()
    {
        m_pScene->SetCamera(Vector3f(0.0f, 0.5f, -5.0f), Vector3f(0.0, -0.1f, 1.0f));

        CombinedObject Ground = LoadAndAddModel("../Content/ground.obj");
        CombinedObject Box = LoadAndAddModel("../Content/box.obj", 0.1f);

        Box.pParticle->SetPosition(Vector3f(-1.0f, 0.0f, 0.0f));
        Box.pParticle->SetMass(2.0f);
        Box.pParticle->SetVelocity(Vector3f(1.0f, 1.0f, 0.0f));
        Box.pParticle->SetDamping(0.99f);
    }


    void InitFireworksDemo()
    {
        m_pScene->SetCamera(Vector3f(0.0f, 0.0f, -50.0f), Vector3f(0.0, 0.1f, 1.0f));

        int NumFireworks = 50;

        Model* pModel = m_pRenderingSystem->LoadModel("../Content/box.obj");

        for (int i = 0 ; i < NumFireworks ; i++) {
            CombinedObject CObject;
            CObject.pSceneObject = m_pScene->CreateSceneObject(pModel);
            CObject.pSceneObject->SetScale(0.1f);
            CObject.pParticle = m_physicsSystem.AllocFirework();
            m_sceneObjects.push_back(CObject);
            m_pScene->AddToRenderList(CObject.pSceneObject);
        }
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
        for (std::list<CombinedObject>::iterator it = m_sceneObjects.begin(); it != m_sceneObjects.end(); it++) {
            if (it->pParticle) {
                const Vector3f& NewPos = it->pParticle->GetPosition();
                it->pSceneObject->SetPosition(NewPos);
            }
        }
    }


    CombinedObject LoadAndAddModel(const char* pFilename, float Scale = 1.0f)
    {
        Model* pModel = m_pRenderingSystem->LoadModel(pFilename);
        CombinedObject CObject;
        CObject.pSceneObject = m_pScene->CreateSceneObject(pModel);
        CObject.pSceneObject->SetScale(Scale);
        CObject.pParticle = m_physicsSystem.AllocParticle();
        m_sceneObjects.push_back(CObject);
        m_pScene->AddToRenderList(CObject.pSceneObject);
        m_physicsSystem.GetRegistry().Add(CObject.pParticle, &m_gravityForceGenerator);
        m_physicsSystem.GetRegistry().Add(CObject.pParticle, &m_dragForceGenerator);
        return CObject;
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


private:
    float m_count = 0.0f;
    Scene* m_pScene = NULL;
    std::list<CombinedObject> m_sceneObjects;
    DirectionalLight m_dirLight;
    PointLight m_pointLight;
    bool m_leftMousePressed = false;
    bool m_midMousePressed = false;
    SceneObject* m_pickedObject = NULL;
    bool m_showGui = false;
    int m_enableShadowMapping = 1;
    OgldevPhysics::PhysicsSystem m_physicsSystem;
    OgldevPhysics::GravityForceGenerator m_gravityForceGenerator;
    OgldevPhysics::DragForceGenerator m_dragForceGenerator;
};



void carbonara()
{
    Carbonara game;

    game.Start();
}
