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

    Tutorial 52 - Normal Mapping
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "demolition.h"
#include "demolition_base_gl_app.h"


#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

// #define POINT_LIGHT   // works a bit weird...


class NormalMapping : public BaseGLApp {
public:
    NormalMapping() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT)
    {
        m_dirLight.WorldDirection = Vector3f(-1.0f, -1.0f, 0.0f);
        m_dirLight.DiffuseIntensity = 1.0f;
        m_dirLight.AmbientIntensity = 0.1f;

        m_pointLight.WorldPosition = Vector3f(0.25f, 0.25f, 0.0f);
        m_pointLight.DiffuseIntensity = 2.5f;
        m_pointLight.AmbientIntensity = 0.5f;
    }

    ~NormalMapping() {}

    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();
        m_pScene->SetClearColor(Vector4f(0.0f, 1.0f, 0.0f, 0.0f));
        m_pScene->SetCamera(Vector3f(0.0f, 0.0f, -4.5f), Vector3f(0.0f, 0.0f, 1.0f));
        m_pScene->SetCameraSpeed(0.1f);
#ifdef POINT_LIGHT
        m_pScene->GetPointLights().push_back(m_pointLight);
#else
        m_pScene->GetDirLights().push_back(m_dirLight);
#endif     
        m_pScene->GetConfig()->ControlShadowMapping(false);

        m_pModel = m_pRenderingSystem->LoadModel("../Content/sphere.obj");
        m_pSceneObject = m_pScene->CreateSceneObject(m_pModel);
        int ColorTexture = m_pRenderingSystem->LoadTexture2D("../Content/dry-rocky-ground-bl/dry-rocky-ground_albedo.png");
        m_pModel->SetColorTexture(ColorTexture);
        m_normalMap = m_pRenderingSystem->LoadTexture2D("../Content/dry-rocky-ground-bl/dry-rocky-ground_normal-ogl.png");
        m_pModel->SetNormalMap(m_normalMap);
        m_pScene->AddToRenderList(m_pSceneObject);

        m_pRenderingSystem->SetScene(m_pScene);

        m_pRenderingSystem->Execute();
    }


    void OnFrame(long long DeltaTimeMillis)
    {
        BaseGLApp::OnFrame(DeltaTimeMillis);      
        m_count += 0.01f;
        m_pSceneObject->ResetRotations();
        m_pSceneObject->PushRotation(Vector3f(0.0f, m_count * 5.0f, 0.0f));

#ifdef POINT_LIGHT      
        m_pScene->GetPointLights()[0].WorldPosition.x = sinf(m_count) * 3.0f;
        m_pScene->GetPointLights()[0].WorldPosition.y = 0.0f;
        m_pScene->GetPointLights()[0].WorldPosition.z = cosf(m_count) * 3.0f;
#else
        m_pScene->GetDirLights()[0].WorldDirection = Vector3f(sinf(m_count), cosf(m_count) * 0.5f, 1.0f);
#endif
    }


    bool OnKeyboard(int key, int action)
    {
        bool HandledByMe = true;

        if (!BaseGLApp::OnKeyboard(key, action)) {
            switch (key) {
            case GLFW_KEY_N:
                if (action == GLFW_PRESS) {
                    m_isNormalMapping = !m_isNormalMapping;

                    if (m_isNormalMapping) {
                        m_pModel->SetNormalMap(m_normalMap);
                    }
                    else {
                        m_pModel->SetNormalMap(-1);
                    }
                }
                break;

            case GLFW_KEY_H:
                m_rotationAngle += 1.0f;
                break;

            case GLFW_KEY_J:
                m_rotationAngle -= 1.0f;
                break;

            default:
                HandledByMe = false;
            }
        }

        return HandledByMe;
    }

private:
    float m_count = 0.0f;
    Scene* m_pScene = NULL;
    SceneObject* m_pSceneObject = NULL;
    DirectionalLight m_dirLight;
    PointLight m_pointLight;
    int m_normalMap = -1;
    bool m_isNormalMapping = false;
    Model* m_pModel = NULL;
    float m_rotationAngle = 0.0f;
};



int main(int argc, char* argv[])
{
    NormalMapping game;

    game.Start();

    return 0;
}
