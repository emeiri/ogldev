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

#pragma once

#include <vector>

#include "ogldev_math_3d.h"
#include "particle.h"

namespace OgldevPhysics
{

class ParticleContact
{
public:

    float CalcSeparatingVelocity() const;

    float GetPenetration() const { return m_penetration; }
    void SetPenetration(float Penetration) { m_penetration = Penetration; }

    void SetRestitution(float Restitution) { m_restitution = Restitution; }

    void Resolve(float dt);

    void SetContactNormal(Vector3f& Normal) { m_contactNormal = Normal; }

    Particle* m_pParticles[2] = { NULL, NULL };    

private:

    void ResolveVelocity(float dt);

    void ResolveInterpenetration(float dt);

    float CalcSepVelocityCausedByAccel(float dt);

    float CalcTotalReciprocalMass();

    float m_restitution = 0.0f;

    Vector3f m_contactNormal;

    float m_penetration = 0.0f;

    Vector3f m_particleMovement[2] = { 0.0f };
};


class ParticleContactResolver {

public:

    ParticleContactResolver(int Iterations);

    void SetIterations(int Iterations) { m_iterations = Iterations; }

    void ResolveContacts(std::vector<ParticleContact>& ContactArray, float dt);

    int FindContactWithLargestClosingVelocity(std::vector<ParticleContact>& ContactArray);

protected:

    int m_iterations = 0;
    int m_iterationsUsed = 0;
};


class ParticleContactGenerator {
public:

    virtual int AddContact(ParticleContact& Contact, int Limit) const = 0;
};


class ParticleLink : public ParticleContactGenerator {

public:
    
    Particle* m_pParticles[2];

    virtual int AddContact(ParticleContact& Contact, int Limit) const = 0;

protected:

    float GetLength() const;
};


class ParticleCable : public ParticleLink {

public:

    virtual int AddContact(ParticleContact& Contact, int Limit) const;

private:

    float m_maxLength = 0.0f;

    float m_restituion = 0.0f;
};

}