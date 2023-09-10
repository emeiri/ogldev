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


class ObjectTest : public GameCallbacks
{
public:

    virtual ~ObjectTest()
    {        
    }


    void Init()
    {
        bool LoadBasicShapes = true;
        m_pRenderingSystem = RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM_GL, this, LoadBasicShapes);
        m_pRenderingSystem->CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

        m_pScene = m_pRenderingSystem->CreateEmptyScene();

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

        m_pSceneObject1->SetRotation(0.0f, m_counter, 0.0f);
        m_pSceneObject2->SetRotation(0.0f, m_counter, 0.0f);
        m_pSceneObject3->SetRotation(0.0f, m_counter, 0.0f);
        m_pSceneObject4->SetRotation(0.0f, m_counter, 0.0f);
    }

private:

    void InitMesh()
    {
        m_pSceneObject1 = m_pScene->CreateSceneObject("cube");
        m_pScene->AddToRenderList(m_pSceneObject1);
        m_pSceneObject1->SetPosition(0.0f, 0.0f, 10.0f);
        m_pSceneObject1->SetFlatColor(Vector4f(1.0f, 0.0, 0.0, 1.0f));

        m_pSceneObject2= m_pScene->CreateSceneObject("cube");
        m_pScene->AddToRenderList(m_pSceneObject2);
        m_pSceneObject2->SetPosition(10.0f, 0.0f, 0.0f);
        m_pSceneObject2->SetFlatColor(Vector4f(1.0f, 1.0, 0.0, 1.0f));

        m_pSceneObject3 = m_pScene->CreateSceneObject("cube");
        m_pScene->AddToRenderList(m_pSceneObject3);
        m_pSceneObject3->SetPosition(-10.0f, 0.0f, 0.0f);
        m_pSceneObject3->SetFlatColor(Vector4f(0.0f, 1.0, 1.0, 1.0f));

        m_pSceneObject4 = m_pScene->CreateSceneObject("cube");
        m_pScene->AddToRenderList(m_pSceneObject4);
        m_pSceneObject4->SetPosition(0.0f, 0.0f, -10.0f);
        m_pSceneObject4->SetFlatColor(Vector4f(1.0f, 0.0, 1.0, 1.0f));

    }

    RenderingSystem* m_pRenderingSystem = NULL;
    Scene* m_pScene = NULL;
    SceneObject* m_pSceneObject1 = NULL;
    SceneObject* m_pSceneObject2 = NULL;
    SceneObject* m_pSceneObject3 = NULL;
    SceneObject* m_pSceneObject4 = NULL;
    float m_counter = 0;    
};


void test_object()
{
    ObjectTest App;
    App.Init();
    App.Run();
}
