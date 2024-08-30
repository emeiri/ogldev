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

#include "contact_resolver.h"

namespace OgldevPhysics
{

void ParticleContact::Resolve(float dt)
{
    ResolveVelocity(dt);
    ResolveInterpenetration(dt);
}


float ParticleContact::CalcSeparatingVelocity() const
{
    Vector3f RelVelocity = m_pParticles[0]->GetVelocity();

    if (m_pParticles[1]) {
        RelVelocity -= m_pParticles[1]->GetVelocity();
    }

    float res = RelVelocity.Dot(m_contactNormal);

    return res;
}


void ParticleContact::ResolveInterpenetration(float dt)
{

}


void ParticleContact::ResolveVelocity(float dt)
{
    float SeparatingVelocity = CalcSeparatingVelocity();

    if (SeparatingVelocity <= 0.0f) {
        float NewSeparatingVelocity = -SeparatingVelocity * m_restitution;

        float DeltaVelocity = NewSeparatingVelocity - SeparatingVelocity;

        float TotalReciprocalMass = m_pParticles[0]->GetReciprocalMass();
        
        if (m_pParticles[1]) {
            TotalReciprocalMass += m_pParticles[1]->GetReciprocalMass();
        }

        if (TotalReciprocalMass > 0.0f) {
            float Impluse = DeltaVelocity / TotalReciprocalMass;

            Vector3f ImpulsePerMass = m_contactNormal * Impluse;

            Vector3f NewVelocity = m_pParticles[0]->GetVelocity() + ImpulsePerMass * m_pParticles[0]->GetReciprocalMass();

            m_pParticles[0]->SetVelocity(NewVelocity);

            if (m_pParticles[1]) {
                NewVelocity = m_pParticles[1]->GetVelocity() + ImpulsePerMass * -m_pParticles[1]->GetReciprocalMass();

                m_pParticles[1]->SetVelocity(NewVelocity);
            }
        }
    }
}


}