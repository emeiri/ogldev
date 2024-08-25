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
#include "bungee_spring_force_generator.h"


namespace OgldevPhysics {

BungeeSpringForceGenerator::BungeeSpringForceGenerator(Particle* pOtherEnd, float SpringConstant, float RestLength)
{
    m_pOtherEnd = pOtherEnd;
    m_springConstant = SpringConstant;
    m_restLength = RestLength;
}


void BungeeSpringForceGenerator::Init(Particle* pOtherEnd, float SpringConstant, float RestLength)
{
    m_pOtherEnd = pOtherEnd;
    m_springConstant = SpringConstant;
    m_restLength = RestLength;
}


void BungeeSpringForceGenerator::UpdateForce(Particle* pParticle, float dt)
{
    Vector3f Force = pParticle->GetPosition();

    Force -= m_pOtherEnd->GetPosition();

    float Magnitude = Force.Length();

    if (Magnitude > m_restLength) {
        Magnitude = m_springConstant * (m_restLength - Magnitude);

        Force = Force.Normalize() * (-Magnitude);

        pParticle->AddForce(Force);
    }
}

}