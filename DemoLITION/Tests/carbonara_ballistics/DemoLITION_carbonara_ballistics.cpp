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

    DemoLITION - Carbonara - BallisticsDemo
*/

#include "../carbonara/carbonara.h"


class BallisticsDemo : public Carbonara {

public:

    BallisticsDemo()
    {
        m_physicsSystem.Init(NUM_PSOs, 1, 1);
    }

    void InitChild()
    {
        m_pScene->SetCamera(Vector3f(0.0f, 0.5f, -5.0f), Vector3f(0.0, -0.1f, 1.0f));

        PhysicsSceneObject Ground = LoadAndAddModel("../Content/ground.obj", true);
        PhysicsSceneObject Box = LoadAndAddModel("../Content/box.obj", true, 0.1f);

        Box.pParticle->SetPosition(Vector3f(-1.0f, 0.0f, 0.0f));
        Box.pParticle->SetMass(2.0f);
        Box.pParticle->SetVelocity(Vector3f(1.0f, 1.0f, 0.0f));
        //   Box.pParticle->SetDamping(0.99f);
    }
};


int main(int argc, char* arg[])
{
    BallisticsDemo demo;

    demo.Start();
}
