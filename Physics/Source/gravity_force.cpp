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
#include "gravity_force_generator.h"


namespace OgldevPhysics {

static Vector3f DEFAULT_GRAVITY = Vector3f(0.0f, -1.0f, 0.0f);

GravityForceGenerator::GravityForceGenerator()
{
    m_gravity = DEFAULT_GRAVITY;
}


GravityForceGenerator::GravityForceGenerator(const Vector3f& Gravity)
{
    m_gravity = Gravity;
}

    
void GravityForceGenerator::UpdateForce(Particle* pParticle, float dt)
{
    if (!pParticle->HasFiniteMass()) {
        return;
    }

    Vector3f Force = m_gravity * pParticle->GetMass();

    pParticle->AddForce(Force);
}

}