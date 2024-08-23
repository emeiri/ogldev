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
#include "drag_force_generator.h"


namespace OgldevPhysics {

DragForceGenerator::DragForceGenerator()
{
}


DragForceGenerator::DragForceGenerator(float k1, float k2)
{
    m_k1 = k1;
    m_k2 = k2;
}

    
void DragForceGenerator::UpdateForce(Particle* pParticle, float dt)
{
    Vector3f Velocity = pParticle->GetVelocity();

    float DragCoeff = Velocity.Length();

    if (DragCoeff > 0.0f) {
        DragCoeff = m_k1 * DragCoeff + m_k2 * DragCoeff * DragCoeff;

        Vector3f Force = Velocity.Normalize() * (-DragCoeff);

        pParticle->AddForce(Force);
    }
}

}