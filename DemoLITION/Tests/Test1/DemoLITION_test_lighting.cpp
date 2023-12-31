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

    DemoLITION - Forward Renderer Demo
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "demolition.h"


#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class LightingTest : public GameCallbacks
{
public:

    virtual ~LightingTest()
    {        
    }


    void Init()
    {
        bool LoadBasicShapes = false;
        m_pRenderingSystem = RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM_GL, this, LoadBasicShapes);
        m_pRenderingSystem->CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

        m_pScene = m_pRenderingSystem->CreateEmptyScene();
        m_pScene->SetClearColor(Vector4f(0.0f, 1.0f, 0.0f, 0.0f));
        m_pRenderingSystem->SetScene(m_pScene);
        
        InitLights();                

        InitObjects();
    }


    void Run()
    {
        m_pRenderingSystem->Execute();
    }

    void OnFrame()
    {
        m_counter += 0.03f;
        m_pScene->GetDirLights()[0].WorldDirection = Vector3f(sinf(m_counter), 0.0f, cosf(m_counter));
        m_pScene->GetPointLights()[0].WorldPosition = Vector3f(1 - sinf(m_counter), 1.0f, 1.0f + cosf(m_counter));
    }

private:

    void InitLights()
    {
        m_dirLight.WorldDirection = Vector3f(1.0f, -0.5f, 0.0f);
        m_dirLight.DiffuseIntensity = 0.5f;
        m_dirLight.AmbientIntensity = 0.1f;
        m_pScene->GetDirLights().push_back(m_dirLight);

        m_pointLight.DiffuseIntensity = 0.3f;
        m_pointLight.Color = Vector3f(1.0f, 1.0f, 1.0f);

        m_pScene->GetPointLights().push_back(m_pointLight);
    }

    void InitObjects()
    {
        m_terrainModelHandle = m_pRenderingSystem->LoadModel("../Content/antique_ceramic_vase_01_4k.blend/antique_ceramic_vase_01_4k.obj");
        m_pTerrainSceneObject = m_pScene->CreateSceneObject(m_terrainModelHandle);
        m_pTerrainSceneObject->SetPosition(0.0f, 0.0f, 4.0f);
        m_pScene->AddToRenderList(m_pTerrainSceneObject);
    }

    RenderingSystem* m_pRenderingSystem = NULL;
    Scene* m_pScene = NULL;
    int m_terrainModelHandle = -1;
    SceneObject* m_pTerrainSceneObject = NULL;
    DirectionalLight m_dirLight;
    PointLight m_pointLight;
    float m_counter = 0;    
};


void test_lighting()
{
    LightingTest App;
    App.Init();
    App.Run();
}
