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

    Tutorial 60 - Reflection & Refraction
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "demolition.h"
#include "demolition_base_gl_app.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080



class Tutorial60 : public BaseGLApp {
public:
    Tutorial60() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 60")
    {
        //  m_dirLight.WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_dirLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        m_dirLight.DiffuseIntensity = 0.9f;
        m_dirLight.AmbientIntensity = 0.1f;
    }

    ~Tutorial60() {}


    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();

        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

        //  m_pScene->SetCameraSpeed(0.1f);

        m_pScene->GetDirLights().push_back(m_dirLight);
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = false;
        m_pScene->GetConfig()->ControlSkybox(true);
        m_pScene->GetConfig()->ControlShadowMapping(false);

        m_pRenderingSystem->SetScene(m_pScene);

        m_pScene->SetCamera(Vector3f(0.0f, 2.0f, -4.0f), Vector3f(0.0, -0.2f, 1.0f));

        //Model* pModel = m_pRenderingSystem->LoadModel("../Content/crystal_ball/scene.gltf");
        //Model* pModel = m_pRenderingSystem->LoadModel("../Content/stanford_armadillo_pbr/scene.gltf");
        Model* pModel = m_pRenderingSystem->LoadModel("../Content/rubber_duck/scene.gltf");
        m_pSceneObject = m_pScene->CreateSceneObject(pModel);
      //  m_pSceneObject->SetScale(Vector3f(0.1f));
     //   m_pSceneObject->SetRotation(90.0f, 0.0f, 0.0f);
        m_pScene->AddToRenderList(m_pSceneObject);

        m_pScene->LoadSkybox("../Content/textures/ahornsteig_4k.jpg");

        m_pRenderingSystem->Execute();
    }


    void OnFrame(long long DeltaTimeMillis)
    {        
        //  if (m_pScene->GetDirLights().size() > 0) {
        //      m_pScene->GetDirLights()[0].WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        //  }

     //   m_pSceneObject->RotateBy(0.0f, 0.5f, 0.0f);
        
        m_count += 1.0f;

        if (m_pScene->GetPickedSceneObject()) {
            m_pickedObject = m_pScene->GetPickedSceneObject();
            m_pickedObject->SetColorMod(2.0f, 1.0f, 1.0f);
        }
        else {
            if (m_pickedObject) {
                m_pickedObject->SetColorMod(1.0f, 1.0f, 1.0f);
                m_pickedObject = NULL;
            }
        }

        m_pSceneObject->ResetRotations();
        m_pSceneObject->PushRotation(Vector3f(180.0f, 0.0f, 0.0f));
        m_pSceneObject->PushRotation(Vector3f(0.0f, 0.0f, m_count));

         //   m_pScene->GetPointLights()[0].WorldPosition.x = sinf(m_count);
          //  m_pScene->GetPointLights()[0].WorldPosition.z = cosf(m_count);
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
    DirectionalLight m_dirLight;
    bool m_leftMousePressed = false;
    bool m_midMousePressed = false;
    SceneObject* m_pickedObject = NULL;
    SceneObject* m_pSceneObject = NULL;
    bool m_showGui = false;
    int m_enableShadowMapping = 1;
};



int main(int argc, char* arg[])
{
    Tutorial60 demo;
    demo.Start();
}
