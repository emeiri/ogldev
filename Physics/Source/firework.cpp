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

 */

#include "ogldev_physics.h"
#include "firework.h"

namespace OgldevPhysics
{

bool Firework::Update(float dt)
{
    Integrate(dt);

    m_age -= dt;

   // printf("age %f\n", m_age);

    bool ret = ((m_age < 0.0f) || (m_position.y < 0));

    return ret;
}


void FireworkConfig::Create(Firework& firework, const Firework* pParent) const
{    
    firework.SetType(m_type);
    firework.SetAge(RandomFloatRange(m_minAge, m_maxAge));

    Vector3f Velocity(0.0f, 0.0f, 0.0f);

    if (pParent) {
        // The position and velocity are based on the parent.
        firework.SetPosition(pParent->GetPosition());
        Velocity += pParent->GetVelocity();
    } else {
        printf("Set type of %p to %d\n", pParent, m_type);
        Vector3f Start(0.0f, 0.0f, 0.0f);
        int x = 1;// (RANDOM() % 3) - 1;
        Start.x = 0.0f * (float)x;
        firework.SetPosition(Start);
    }

    Vector3f RandomVelocity;
    RandomVelocity.InitRandom(m_minVelocity, m_maxVelocity);
    Velocity += RandomVelocity;
    firework.SetVelocity(Velocity);

    // We use a mass of one in all cases (no point having fireworks
    // with different masses, since they are only under the influence
    // of gravity).
    firework.SetMass(1.0f);

    firework.SetDamping(m_damping);

    firework.SetAcceleration(GRAVITY);

   // firework.ClearAccumulator();
}


void PhysicsSystem::InitFireworksConfig()
{
    m_fireworkConfigs.resize(9);

    m_fireworkConfigs[0].Init(2);
    m_fireworkConfigs[0].SetParams(
        1, // type
        0.5f, 1.4f, // age range
        Vector3f(-5, 25, -5), // min velocity
        Vector3f(5, 28, 5), // max velocity
        0.1f // damping
    );
    m_fireworkConfigs[0].m_payloads[0].Set(3, 5);
    m_fireworkConfigs[0].m_payloads[1].Set(5, 5);

    m_fireworkConfigs[1].Init(1);
    m_fireworkConfigs[1].SetParams(
        2, // type
        0.5f, 1.0f, // age range
        Vector3f(-5, 10, -5), // min velocity
        Vector3f(5, 20, 5), // max velocity
        0.8f // damping
    );
    m_fireworkConfigs[1].m_payloads[0].Set(4, 2);

    m_fireworkConfigs[2].Init(0);
    m_fireworkConfigs[2].SetParams(
        3, // type
        0.5f, 1.5f, // age range
        Vector3f(-5, -5, -5), // min velocity
        Vector3f(5, 5, 5), // max velocity
        0.1f // damping
    );

    m_fireworkConfigs[3].Init(0);
    m_fireworkConfigs[3].SetParams(
        4, // type
        0.25f, 0.5f, // age range
        Vector3f(-20, 5, -5), // min velocity
        Vector3f(20, 5, 5), // max velocity
        0.2f // damping
    );

    m_fireworkConfigs[4].Init(1);
    m_fireworkConfigs[4].SetParams(
        5, // type
        0.5f, 1.0f, // age range
        Vector3f(-20, 2, -5), // min velocity
        Vector3f(20, 18, 5), // max velocity
        0.01f // damping
    );
    m_fireworkConfigs[4].m_payloads[0].Set(3, 5);

    m_fireworkConfigs[5].Init(0);
    m_fireworkConfigs[5].SetParams(
        6, // type
        3, 5, // age range
        Vector3f(-5, 5, -5), // min velocity
        Vector3f(5, 10, 5), // max velocity
        0.95f // damping
    );

    m_fireworkConfigs[6].Init(1);
    m_fireworkConfigs[6].SetParams(
        7, // type
        4, 5, // age range
        Vector3f(-5, 50, -5), // min velocity
        Vector3f(5, 60, 5), // max velocity
        0.01f // damping
    );
    m_fireworkConfigs[6].m_payloads[0].Set(8, 10);

    m_fireworkConfigs[7].Init(0);
    m_fireworkConfigs[7].SetParams(
        8, // type
        0.25f, 0.5f, // age range
        Vector3f(-1, -1, -1), // min velocity
        Vector3f(1, 1, 1), // max velocity
        0.01f // damping
    );

    m_fireworkConfigs[8].Init(0);
    m_fireworkConfigs[8].SetParams(
        9, // type
        3, 5, // age range
        Vector3f(-15, 10, -5), // min velocity
        Vector3f(15, 15, 5), // max velocity
        0.95f // damping
    );

    Create(1, 1, NULL);
}


void PhysicsSystem::Create(int Type, uint Count, OgldevPhysics::Firework* pFirework)
{
    for (uint i = 0; i < Count; i++) {
        OgldevPhysics::FireworkConfig& Config = m_fireworkConfigs[Type - 1];
        Config.Create(m_fireworks[m_nextFirework], pFirework);
        m_nextFirework = (m_nextFirework + 1) % m_fireworks.size();
    }
}



}