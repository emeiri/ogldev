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


#include "particle.h"

namespace OgldevPhysics
{

void Particle::Integrate(float dt)
{
    if (m_reciprocalMass <= 0.0f) {
        return;
    }

    m_position += m_velocity * dt;

    Vector3f AccTemp = m_acceleration;
    AccTemp += m_forceAccum * m_reciprocalMass;
   // printf("accel y %f\n", AccTemp.y);
    m_velocity += AccTemp * dt;

    m_velocity *= powf(m_damping, dt);

  //  printf("pos y %f velocity y %f\n", m_position.y, m_velocity.y);

    ClearAccum();
}


void Particle::SetMass(float Mass)
{
    assert(Mass > 0.0f);

    m_reciprocalMass = 1.0f / Mass;
}


float Particle::GetMass() const
{
    float ret = 0.0f;

    if (m_reciprocalMass == 0.0f) {
        ret = FLT_MAX;
    } else {
        ret = 1.0f / m_reciprocalMass;
    }

    return ret;
}


void Particle::ClearAccum()
{
    m_forceAccum = Vector3f(0.0f, 0.0f, 0.0f);
}

}