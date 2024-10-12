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

#include <assert.h>
#include <algorithm>

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
    if (m_penetration > 0.0f) {
        float TotalReciprocalMass = m_pParticles[0]->GetReciprocalMass();

        if (m_pParticles[1]) {
            TotalReciprocalMass += m_pParticles[1]->GetReciprocalMass();

            if (TotalReciprocalMass > 0.0f) {
                Vector3f MovePerReciprocalMass = m_contactNormal * m_penetration / TotalReciprocalMass;

                m_particleMovement[0] = MovePerReciprocalMass * m_pParticles[0]->GetReciprocalMass();

                Vector3f NewPos = m_pParticles[0]->GetPosition() + m_particleMovement[0];
                m_pParticles[0]->SetPosition(NewPos);

                if (m_pParticles[1]) {
                    m_particleMovement[1] = MovePerReciprocalMass * m_pParticles[1]->GetReciprocalMass();

                    NewPos = m_pParticles[1]->GetPosition() + m_particleMovement[1];
                    m_pParticles[1]->SetPosition(NewPos);
                }
            }
        }
    }
}


void ParticleContact::ResolveVelocity(float dt)
{
    float SeparatingVelocity = CalcSeparatingVelocity();

    if (SeparatingVelocity <= 0.0f) {
        float NewSepVelocity = -SeparatingVelocity * m_restitution;

        float SepVelocityCausedByAccel = CalcSepVelocityCausedByAccel(dt);

        if (SepVelocityCausedByAccel < 0) {
            NewSepVelocity += SepVelocityCausedByAccel * m_restitution;

            NewSepVelocity = std::max(0.0f, NewSepVelocity);
        }

        float DeltaVelocity = NewSepVelocity - SeparatingVelocity;

        float TotalReciprocalMass = CalcTotalReciprocalMass();

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


float ParticleContact::CalcSepVelocityCausedByAccel(float dt)
{
    Vector3f VelocityCausedByAccel = m_pParticles[0]->GetAcceleration();

    if (m_pParticles[1]) {
        VelocityCausedByAccel -= m_pParticles[1]->GetAcceleration();
    }

    float SepVelocityCausedByAccel = VelocityCausedByAccel.Dot(m_contactNormal) * dt;

    return SepVelocityCausedByAccel;
}


float ParticleContact::CalcTotalReciprocalMass()
{
    float TotalReciprocalMass = m_pParticles[0]->GetReciprocalMass();

    if (m_pParticles[1]) {
        TotalReciprocalMass += m_pParticles[1]->GetReciprocalMass();
    }

    return TotalReciprocalMass;
}


void ParticleContactResolver::ResolveContacts(std::vector<ParticleContact>& ContactArray, float dt)
{
    int i = 0;

    int IterationsUsed = 0;

    while (IterationsUsed < m_iterations) {
        int Index = FindContactWithLargestClosingVelocity(ContactArray);

        if (Index == ContactArray.size()) {
            break;
        }

        ContactArray[i].Resolve(dt);

        IterationsUsed++;
    }
}


int ParticleContactResolver::FindContactWithLargestClosingVelocity(std::vector<ParticleContact>& ContactArray)
{
    float MaxSepVelocity = FLT_MAX;
    int MaxIndex = (int)ContactArray.size();

    for (int i = 0; i < ContactArray.size(); i++) {
        float SepVelocity = ContactArray[i].CalcSeparatingVelocity();

        if ((SepVelocity < MaxSepVelocity) && ((SepVelocity < 0.0f) || (ContactArray[i].GetPenetration() > 0.0f))) {
            MaxSepVelocity = SepVelocity;
            MaxIndex = i;
        }
    }

    return MaxIndex;
}


float ParticleLink::GetLength() const
{
    assert(m_pParticles[0]);
    assert(m_pParticles[1]);

    Vector3f RelativePos = m_pParticles[0]->GetPosition() - m_pParticles[1]->GetPosition();

    float Len = RelativePos.Length();

    return Len;
}


int ParticleCable::AddContact(ParticleContact& Contact, int Limit) const
{
    float Length = GetLength();

    float Ret = 0.0f;

    if (Length >= m_maxLength) {
        Contact.m_pParticles[0] = m_pParticles[0];
        Contact.m_pParticles[1] = m_pParticles[1];

        Vector3f Normal = m_pParticles[1]->GetPosition() - m_pParticles[0]->GetPosition();

        Normal.Normalize();

        Contact.SetContactNormal(Normal);
        Contact.SetPenetration(Length - m_maxLength);
        Contact.SetRestitution(m_restituion);
    }

    return 0;
}

}