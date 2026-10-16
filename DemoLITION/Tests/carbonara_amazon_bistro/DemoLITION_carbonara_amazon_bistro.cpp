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

    DemoLITION - Carbonara - AmazonBistroDemo
*/

#include "../carbonara/carbonara.h"


class AmazonBistroDemo : public Carbonara {

public:

    AmazonBistroDemo()
    {
    }

    void InitChild()
    {
        m_pScene->GetConfig()->ControlShadowMapping(false);
        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

        DirectionalLight DirLight;
        DirLight.Color = Vector3f(1.0f);
        DirLight.WorldDirection = Vector3f(1.0f, -1.0f, 0.0f);
        DirLight.DiffuseIntensity = 0.8f;
        DirLight.AmbientIntensity = 0.2f;
        m_pScene->GetDirLights().clear();
        m_pScene->GetPointLights().clear();
        m_pScene->GetSpotLights().clear();
        m_pScene->GetDirLights().push_back(DirLight);

        //Model* pModel = m_pRenderingSystem->LoadModel("C:/Users/emeir/Downloads/Bistro_v5_2/Bistro_v5_2/BistroExterior.fbx");
         Model* pModel = m_pRenderingSystem->LoadModel("G:/Models/McGuire/bistro/Exterior/exterior.obj");
       
        SceneObject* pSceneObject = m_pScene->CreateSceneObject(pModel);
        pSceneObject->SetScale(0.01f);
        m_pScene->AddToRenderList(pSceneObject);

        m_pScene->SetCamera(Vector3f(-2.5f, 3.0f, 6.0f), Vector3f(1.0f, -0.1f, 0.25f));        
    }
};


int main(int argc, char* arg[])
{
    AmazonBistroDemo demo;

    demo.Start();
}
