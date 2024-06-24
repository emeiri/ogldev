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


class ParallaxDemo : public BaseGLApp {
public:
    ParallaxDemo() : BaseGLApp(WINDOW_WIDTH, WINDOW_HEIGHT)
    {
      //  m_dirLight.WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_dirLight.WorldDirection = Vector3f(0.0f, -1.0f, -1.0f);
        m_dirLight.DiffuseIntensity = 1.0f;

        m_pointLight.WorldPosition = Vector3f(0.25f, 0.25f, 0.0f);
     //  m_pointLight.WorldPosition = Vector3f(1.0f, 0.0f, -1.0f);
        m_pointLight.DiffuseIntensity = 2.0f;
        m_pointLight.AmbientIntensity = 0.1f;
    }

    ~ParallaxDemo() {}

    void Start()
    {
        m_pScene = m_pRenderingSystem->CreateEmptyScene();

      //  Model* pModel = m_pRenderingSystem->LoadModel("../Content/dry-rocky-ground-bl/dry-rocky-ground-bl.obj");
        //Model* pModel = m_pRenderingSystem->LoadModel("../Content/mybrick/mybrick.obj");
         Model* pModel = m_pRenderingSystem->LoadModel("../Content/brickwall2.obj");
       // Model* pModel = pRenderingSystem->LoadModel("../Content/brickwall.obj");
        m_pSceneObject = m_pScene->CreateSceneObject(pModel);
      //  m_pSceneObject->SetPosition(0.0f, 0.0f, 4.0f);
        //pSceneObject->SetRotation(Vector3f(-90.0f, 0.0f, 0.0f));
        //m_pSceneObject->SetRotation(Vector3f(0.0f, 65.0f, 0.0f));
        m_pScene->AddToRenderList(m_pSceneObject);

       // int NormalMap = m_pRenderingSystem->LoadTexture2D("../Content/dry-rocky-ground-bl/dry-rocky-ground_normal-ogl.png");
        //int NormalMap = m_pRenderingSystem->LoadTexture2D("../Content/mybrick/mybrick-normal.png");
       // int NormalMap = pRenderingSystem->LoadTexture2D("../Content/brickwall_normal.jpg");
        int NormalMap = m_pRenderingSystem->LoadTexture2D("../Content/bricks2_normal.jpg");
        
        pModel->SetNormalMap(NormalMap);

       // int HeightMap = m_pRenderingSystem->LoadTexture2D("../Content/dry-rocky-ground-bl/dry-rocky-ground_height.png");
        //int HeightMap = m_pRenderingSystem->LoadTexture2D("../Content/mybrick/mybrick-height.png");
        int HeightMap = m_pRenderingSystem->LoadTexture2D("../Content/bricks2_disp.jpg");
        pModel->SetHeightMap(HeightMap);

        m_pScene->SetClearColor(Vector4f(0.0f, 1.0f, 0.0f, 0.0f));

        /* DirectionalLight l;
         l.WorldDirection = Vector3f(1.0f, -0.5f, 0.0f);
         l.DiffuseIntensity = 0.5f;
         l.AmbientIntensity = 0.1f;
         pScene->GetDirLights().push_back(l);*/

        m_pScene->SetCamera(Vector3f(0.0f, 1.0f, -2.5f), Vector3f(0.000823f, -0.331338f, 0.943512f));
        m_pScene->SetCameraSpeed(0.1f);

        //m_pScene->GetDirLights().push_back(m_dirLight);
        m_pScene->GetPointLights().push_back(m_pointLight);

        m_pRenderingSystem->SetScene(m_pScene);

        m_pRenderingSystem->Execute();
    }


    void OnFrame()
    {
        BaseGLApp::OnFrame();
      //  m_pScene->GetDirLights()[0].WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_count += 0.01f;
        m_pSceneObject->ResetRotations();
        m_pSceneObject->PushRotation(Vector3f(-90.0f, 0.0f, 0.0f));
        //m_pSceneObject->PushRotation(Vector3f(0.0f, 90.0f, 0.0f));

        m_pScene->GetPointLights()[0].WorldPosition.x = sinf(m_count);
        m_pScene->GetPointLights()[0].WorldPosition.z = cosf(m_count);
    }

private:
    float m_count = 0.0f;
    Scene* m_pScene = NULL;
    SceneObject* m_pSceneObject = NULL;
    DirectionalLight m_dirLight;
    PointLight m_pointLight;
};



int main(int argc, char* argv[])
{
    ParallaxDemo game;

    game.Start();

    return 0;
}
