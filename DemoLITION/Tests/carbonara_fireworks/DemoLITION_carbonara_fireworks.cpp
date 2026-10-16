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

    DemoLITION - Carbonara - FireworksDemo
*/

#include "../carbonara/carbonara.h"


class FireworksDemo : public Carbonara {

public:

    FireworksDemo()  
    {
        m_physicsSystem.Init(NUM_PSOs, 1, 1);
    }

    void InitChild()
    {
        m_pScene->SetCamera(Vector3f(0.0f, 0.0f, -50.0f), Vector3f(0.0, 0.1f, 1.0f));

        int NumFireworks = 50;

        Model* pModel = m_pRenderingSystem->LoadModel("../Content/box.obj");

        for (int i = 0; i < NumFireworks; i++) {
            PhysicsSceneObject CObject;
            CObject.pSceneObject = m_pScene->CreateSceneObject(pModel);
            CObject.pSceneObject->SetScale(0.1f);
            CObject.pParticle = m_physicsSystem.AllocFirework();
            m_sceneObjects.push_back(CObject);
            m_pScene->AddToRenderList(CObject.pSceneObject);
        }
    }
};


int main(int argc, char* arg[])
{
    FireworksDemo demo;

    demo.Start();
}
