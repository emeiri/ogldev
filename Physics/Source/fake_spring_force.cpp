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
#include "fake_spring_force_generator.h"


namespace OgldevPhysics {

FakeSpringForceGenerator::FakeSpringForceGenerator(const Vector3f* pAnchor, float SpringConstant, float Damping)
{
    m_pAnchor = pAnchor;
    m_springConstant = SpringConstant;
    m_damping = Damping;
}


void FakeSpringForceGenerator::Init(const Vector3f* pAnchor, float SpringConstant, float Damping)
{
    m_pAnchor = pAnchor;
    m_springConstant = SpringConstant;
    m_damping = Damping;
}

    
void FakeSpringForceGenerator::UpdateForce(Particle* pParticle, float dt)
{
    if (!pParticle->HasFiniteMass()) {
        return;
    }

    Vector3f Position = pParticle->GetPosition() - *m_pAnchor;

    float Gamma = 0.5f * sqrtf(4.0f * m_springConstant - m_damping * m_damping);

    if (Gamma == 0.0f) {
        return;
    }

    Vector3f c = Position * (m_damping / (2.0f * Gamma)) + pParticle->GetVelocity() * (1.0f / Gamma);

    Vector3f Target = Position * cosf(Gamma * dt) + c * sinf(Gamma * dt);

    Vector3f Acceleration = (Target - Position) * (1.0f / dt * dt) - pParticle->GetVelocity() * dt;

    Vector3f Force = Acceleration * pParticle->GetMass();

    pParticle->AddForce(Force);
}

}