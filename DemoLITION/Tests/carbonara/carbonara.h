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

    DemoLITION - Carbonara (common code of the Carbonara demos)
*/

#pragma once

#include <stdio.h>
#include <math.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imGuIZMOquat.h"

#include "demolition.h"
#include "demolition_base_gl_app.h"
#include "ogldev_physics.h"
#include "ogldev_accelorobject.h"


#define WINDOW_WIDTH  2560
#define WINDOW_HEIGHT 1440

#define NUM_PSOs 1000

// TODO: code duplication with ogldev_imgui.cpp in vulkan and gl_rendering_system. need to share.
static bool IsMouseControlledByImGUI()
{
    ImGuiIO& io = ImGui::GetIO();

    bool ret = io.WantCaptureMouse;

    return ret;
}


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
        m_dirLight[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight[0].WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_dirLight[0].DiffuseIntensity = 0.8f;
        m_dirLight[0].AmbientIntensity = 0.2f;

        m_dirLight[1].Color = Vector3f(1.0f, 0.0f, 0.0f);
        m_dirLight[1].WorldDirection = Vector3f(-1.0f, -1.0f, 0.0f);
        m_dirLight[1].DiffuseIntensity = 1.0f;
        m_dirLight[1].AmbientIntensity = 0.0f;

        m_pointLight[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_pointLight[0].WorldPosition = Vector3f(-10.0f, 10.0f, 0.0f);
        m_pointLight[0].DiffuseIntensity = 1.0f;
        m_pointLight[0].AmbientIntensity = 0.0f;
        m_pointLight[0].Attenuation.Constant = 0.25f;

        m_pointLight[1].Color = Vector3f(0.0f, 0.0f, 1.0f);
        m_pointLight[1].WorldPosition = Vector3f(10.0f, 10.0f, 0.0f);
        m_pointLight[1].DiffuseIntensity = 0.2f;
        m_pointLight[1].AmbientIntensity = 0.0f;
        m_pointLight[1].Attenuation.Constant = 0.25f;

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
        
        m_pScene->SetCameraAcceleration(50.0f);

      //  m_pScene->GetConfig()->GetInfiniteGrid().Enabled = true;
        m_pScene->GetDirLights().push_back(m_dirLight[0]);
        //m_pScene->GetPointLights().push_back(m_pointLight[0]);
        //m_pScene->GetPointLights().push_back(m_pointLight[1]);
        //m_pScene->GetSpotLights().push_back(m_spotLight);

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


    void OnFrame(double DeltaTime)
    {
        OnFrameChild(DeltaTime);

        if (m_pScene->GetPointLights().size() > 0) {
            m_pScene->GetPointLights()[0].WorldPosition = Vector3f(sinf(m_count) * 10.0f, 1.0f, cosf(m_count) * 10.0f);

            if (m_pScene->GetPointLights().size() > 1) {
                m_pScene->GetPointLights()[1].WorldPosition = Vector3f(-sinf(m_count) * 10.0f, 1.0f, -cosf(m_count) * 10.0f);
            }
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

        m_physicsSystem.Update(DeltaTime);

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
        Scene* pScene = m_pRenderingSystem->GetScene();    

        pScene->StartSceneGUI("Scene Config");
        ChildGui();
        pScene->EndSceneGUI();
    }


    virtual void ChildGui() {}


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
        bool ret = false;

        if (!IsMouseControlledByImGUI()) {
            ret = !m_leftMousePressed;
        }

        return ret;
    }


    bool OnMouseButton(int Button, int Action, int Mode, int x, int y)
    {
        switch (Button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            m_leftMousePressed = (Action == GLFW_PRESS);
            break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            m_midMousePressed = (Action == GLFW_PRESS);
            m_pScene->GetConfig()->ControlPicking(m_midMousePressed);
            break;
        }

        return false;
    }

protected:

    virtual void OnFrameChild(double DeltaTime) {}

    Scene* m_pScene = NULL;
    std::list<PhysicsSceneObject> m_sceneObjects;
    OgldevPhysics::PhysicsSystem m_physicsSystem;

private:

    float m_count = 0.0f;

    DirectionalLight m_dirLight[2];
    PointLight m_pointLight[2];
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
