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

    DemoLITION - Carbonara - CubeMappingDemo
*/

#include "../carbonara/carbonara.h"


class CubeMappingDemo : public Carbonara {

public:

    CubeMappingDemo()
    {

    }

    void InitChild()
    {
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = false;
        m_pScene->GetConfig()->ControlShadowMapping(false);
        m_pScene->GetConfig()->ControlSkybox(true);
        m_pScene->GetConfig()->ForcePBRDisabled(true);
        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
        m_pScene->SetCamera(Vector3f(0.0f, 1.0f, -2.5f), Vector3f(0.0f, 0.0f, 1.0f));

        Model* pModel = m_pRenderingSystem->LoadModel("../Content/Pixabay/arunangshubanerjee-pool-ball-3-2026.glb");

        SceneObject* pSceneObject = m_pScene->CreateSceneObject(pModel);
        pSceneObject->SetScale(20.0f);
        pSceneObject->SetCubeMapping(true);
        m_pScene->AddToRenderList(pSceneObject);
        //   pSceneObject->SetPosition(0.0f, 0.0f, 10.0f);

        m_pScene->LoadSkybox("../Content/textures/ahornsteig_4k.jpg");
    }
};


int main(int argc, char* arg[])
{
    CubeMappingDemo demo;

    demo.Start();
}
