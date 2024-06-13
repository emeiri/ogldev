/*

        Copyright 2023 Etay Meiri

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

    DemoLITION - Parallax mapping demo
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "demolition.h"


#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class ParallaxDemo : public GameCallbacks {
public:
    ParallaxDemo()
    {
      //  m_dirLight.WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_dirLight.WorldDirection = Vector3f(0.0f, -1.0f, -1.0f);
        m_dirLight.DiffuseIntensity = 1.0f;
    }

    ~ParallaxDemo() {}

    void Init(Scene* pScene)
    {
        m_pScene = pScene;
        pScene->GetDirLights().push_back(m_dirLight);
    }

    void OnFrame()
    {
      //  m_pScene->GetDirLights()[0].WorldDirection = Vector3f(sinf(m_count), -1.0f, cosf(m_count));
        m_count += 0.01f;
    }


    bool OnMouseMove(int x, int y)
    {
        return true;
    }

private:
    float m_count = 0.0f;
    Scene* m_pScene = NULL;
    DirectionalLight m_dirLight;
};



void test_parallax_map()
{
    ParallaxDemo game;

    bool LoadBasicShapes = false;
    RenderingSystem* pRenderingSystem = RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM_GL, &game, LoadBasicShapes);

    pRenderingSystem->CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

    Scene* pScene = pRenderingSystem->CreateEmptyScene();

    //Model* pModel = pRenderingSystem->LoadModel("../Content/dry-rocky-ground-bl/dry-rocky-ground-bl.obj");    
    Model* pModel = pRenderingSystem->LoadModel("../Content/mybrick/mybrick.obj");
    SceneObject* pSceneObject = pScene->CreateSceneObject(pModel);
    pSceneObject->SetPosition(0.0f, 0.0f, 4.0f);
    //pSceneObject->SetRotation(Vector3f(-90.0f, 0.0f, 0.0f));
    pSceneObject->SetRotation(Vector3f(0.0f, 65.0f, 0.0f));
    pScene->AddToRenderList(pSceneObject);

    int NormalMap = pRenderingSystem->LoadTexture2D("../Content/mybrick/mybrick-normal.png");
    pModel->SetNormalMap(NormalMap);            

    int HeightMap = pRenderingSystem->LoadTexture2D("../Content/mybrick/mybrick-height.png");
    pModel->SetHeightMap(HeightMap);

    pScene->SetClearColor(Vector4f(0.0f, 1.0f, 0.0f, 0.0f));

   /* DirectionalLight l;
    l.WorldDirection = Vector3f(1.0f, -0.5f, 0.0f);
    l.DiffuseIntensity = 0.5f;
    l.AmbientIntensity = 0.1f;
    pScene->GetDirLights().push_back(l);*/
    
    pScene->SetCamera(Vector3f(0.0f, 0.6f, 2.0f), Vector3f(0.000823f, -0.331338f, 0.943512f));
    pScene->SetCameraSpeed(0.1f);
    
    game.Init(pScene);

    pRenderingSystem->SetScene(pScene);

    pRenderingSystem->Execute();
}
