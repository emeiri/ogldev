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
        m_pRenderingSubsystem = BaseRenderingSubsystem::CreateRenderingSubsystem(RENDERING_SUBSYSTEM_GL, this, LoadBasicShapes);
        m_pRenderingSubsystem->CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

        m_pScene = m_pRenderingSubsystem->CreateScene();
        DirectionalLight DirLight;
        DirLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        DirLight.DiffuseIntensity = 1.0f;

        m_pScene->m_dirLights.push_back(DirLight);

        m_pScene->SetClearColor(Vector4f(0.0f, 1.0f, 0.0f, 0.0f));

        m_pRenderingSubsystem->SetScene(m_pScene);

        InitMesh();
    }


    void Run()
    {
        m_pRenderingSubsystem->Execute();
    }

    void OnFrame()
    {
        m_counter += 0.1f;

        m_pScene->GetSceneObject(m_sceneObjectHandle1)->SetRotation(0.0f, m_counter, 0.0f);
        m_pScene->GetSceneObject(m_sceneObjectHandle2)->SetRotation(0.0f, m_counter, 0.0f);
        m_pScene->GetSceneObject(m_sceneObjectHandle3)->SetRotation(0.0f, m_counter, 0.0f);
        m_pScene->GetSceneObject(m_sceneObjectHandle4)->SetRotation(0.0f, m_counter, 0.0f);
    }

private:

    void InitMesh()
    {
        m_sceneObjectHandle1 = m_pScene->CreateSceneObject("cube");
        m_pScene->AddToRenderList(m_sceneObjectHandle1);
        m_pScene->GetSceneObject(m_sceneObjectHandle1)->SetPosition(0.0f, 0.0f, 10.0f);
        m_pScene->GetSceneObject(m_sceneObjectHandle1)->SetFlatColor(Vector4f(1.0f, 0.0, 0.0, 1.0f));

        m_sceneObjectHandle2= m_pScene->CreateSceneObject("cube");
        m_pScene->AddToRenderList(m_sceneObjectHandle2);
        m_pScene->GetSceneObject(m_sceneObjectHandle2)->SetPosition(10.0f, 0.0f, 0.0f);
        m_pScene->GetSceneObject(m_sceneObjectHandle2)->SetFlatColor(Vector4f(1.0f, 1.0, 0.0, 1.0f));

        m_sceneObjectHandle3 = m_pScene->CreateSceneObject("cube");
        m_pScene->AddToRenderList(m_sceneObjectHandle3);
        m_pScene->GetSceneObject(m_sceneObjectHandle3)->SetPosition(-10.0f, 0.0f, 0.0f);
        m_pScene->GetSceneObject(m_sceneObjectHandle3)->SetFlatColor(Vector4f(0.0f, 1.0, 1.0, 1.0f));

        m_sceneObjectHandle4 = m_pScene->CreateSceneObject("cube");
        m_pScene->AddToRenderList(m_sceneObjectHandle4);
        m_pScene->GetSceneObject(m_sceneObjectHandle4)->SetPosition(0.0f, 0.0f, -10.0f);
        m_pScene->GetSceneObject(m_sceneObjectHandle4)->SetFlatColor(Vector4f(1.0f, 0.0, 1.0, 1.0f));

    }

    BaseRenderingSubsystem* m_pRenderingSubsystem = NULL;
    Scene* m_pScene = NULL;
    int m_sceneObjectHandle1 = -1;
    int m_sceneObjectHandle2 = -1;
    int m_sceneObjectHandle3 = -1;
    int m_sceneObjectHandle4 = -1;
    float m_counter = 0;    
};


void test_object()
{
    ObjectTest App;
    App.Init();
    App.Run();
}
