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

}