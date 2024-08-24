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
#include "spring_force_generator.h"


namespace OgldevPhysics {

SpringForceGenerator::SpringForceGenerator(Particle* pOtherEnd, float SpringConstant, float RestLength)
{
    m_pOtherEnd = pOtherEnd;
    m_springConstant = SpringConstant;
    m_restLength = RestLength;
}

    
void SpringForceGenerator::UpdateForce(Particle* pParticle, float dt)
{
    Vector3f Velocity = pParticle->GetVelocity();

    Velocity -= m_pOtherEnd->GetPosition();

    float Magnitude = Velocity.Length();

    if (Magnitude > 0.0f) {
        Magnitude = fabsf(Magnitude - m_restLength);

        Magnitude *= m_springConstant;

        Vector3f Force = Velocity.Normalize() * (-Magnitude);

        pParticle->AddForce(Force);
    }
}

}