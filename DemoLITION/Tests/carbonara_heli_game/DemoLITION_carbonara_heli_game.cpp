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

    DemoLITION - Carbonara - HeliGame
*/

#include "../carbonara/carbonara.h"


class HeliGame : public Carbonara {

public:

    HeliGame()
    {
    }

    void InitChild()
    {
        delete m_pScene;

        m_pScene = m_pRenderingSystem->CreateScene("../Content/demolition/dir_light.glb");

        m_pScene->SetClearColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
        m_pScene->GetConfig()->ControlShadowMapping(false);

        m_pRenderingSystem->SetScene(m_pScene);

       // SceneObject* pSceneObject = m_pScene->CreateSceneObject(pModel);
       // pSceneObject->SetScale(0.01f);
      //  m_pScene->AddToRenderList(pSceneObject);

        // Hack: the camera should be set up in the glTF file, but for some reason it is not working, so we set it up here
        m_pScene->SetCamera(Vector3f(0.0f, 10.0f, -30.0f), Vector3f(0.0f, -0.3f, 1.0f));
    }
};


int main(int argc, char* arg[])
{
    HeliGame demo;

    demo.Start();
}
