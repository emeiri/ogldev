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


#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000


class MoveObjectTest : public GameCallbacks
{
public:

    virtual ~MoveObjectTest()
    {
    }


    void Init()
    {
        bool LoadBasicShapes = false;
        m_pRenderingSystem = RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM_GL, this, LoadBasicShapes);

        m_pRenderingSystem->CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

        m_pScene = m_pRenderingSystem->CreateEmptyScene();
        DirectionalLight DirLight;
        DirLight.WorldDirection = Vector3f(0.0f, 0.0f, 1.0f);
        DirLight.DiffuseIntensity = 1.0f;

        m_pScene->GetDirLights().push_back(DirLight);

        m_pScene->SetClearColor(Vector4f(0.0f, 1.0f, 0.0f, 0.0f));

        m_pRenderingSystem->SetScene(m_pScene);

        InitMesh();
    }


    void Run()
    {
        m_pRenderingSystem->Execute();
    }

    void OnFrame()
    {
        m_counter += 0.1f;

        m_pSceneObject->SetRotation(0.0f, m_counter, 0.0f);
    }

    bool OnMouseMove(int x, int y) 
    { 
        m_pSceneObject->SetPosition((float)x / 200.0f, (float)y / 200.0f, 10.0f);
        return true; 
    }

private:

    void InitMesh()
    {
        m_modelHandle = m_pRenderingSystem->LoadModel("../Content/test.glb");
        m_pSceneObject = m_pScene->CreateSceneObject(m_modelHandle);
        m_pScene->AddToRenderList(m_pSceneObject);

        m_pSceneObject->SetPosition(0.0f, 0.0f, 10.0f);
    }

    RenderingSystem* m_pRenderingSystem = NULL;
    Scene* m_pScene = NULL;
    int m_modelHandle = -1;
    SceneObject* m_pSceneObject = NULL;
    float m_counter = 0;    
};


void test_move_object()
{
    MoveObjectTest App;
    App.Init();
    App.Run();
}
