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
    Vector3f m_particleMovement[2] = { 0.0f };
    float m_penetration = 0.0f;
    Vector3f m_contactNormal;

private:

    void ResolveVelocity(float dt);

    void ResolveInterpenetration(float dt);

    float CalcSepVelocityCausedByAccel(float dt);

    float CalcTotalReciprocalMass();

    float m_restitution = 0.0f;
};


class ParticleContactResolver {

public:

    ParticleContactResolver() {}

    void Init(int Iterations) { SetIterations(Iterations); }

    void SetIterations(int Iterations) { m_iterations = Iterations; }

    void ResolveContacts(std::vector<ParticleContact>& ContactArray, uint NumContacts, float dt);

protected:

    int FindContactWithLargestClosingVelocity(std::vector<ParticleContact>& ContactArray, uint NumContacts);

    int m_iterations = 0;
    int m_iterationsUsed = 0;
};


class ParticleContactGenerator {
public:

    virtual int AddContact(std::vector<ParticleContact>& Contacts, int StartIndex) const = 0; // TODO: Contact should be an array of pointers
};


class ParticleLink : public ParticleContactGenerator {

public:
    
    Particle* m_pParticles[2];

    virtual int AddContact(std::vector<ParticleContact>& Contacts, int StartIndex) const = 0;

protected:

    float GetLength() const;
};


class ParticleCable : public ParticleLink {

public:

    virtual int AddContact(std::vector<ParticleContact>& Contacts, int StartIndex) const;

    float m_maxLength = 0.0f;

    float m_restituion = 0.0f;
};


class ParticleRod : public ParticleLink {

public:

    float m_len = 0.0f;

    virtual int AddContact(std::vector<ParticleContact>& Contacts, int StartIndex) const;
};


class ParticleConstraint : public ParticleContactGenerator
{

public:

    Particle* m_pParticle = NULL;
    Vector3f m_anchor;

    virtual int AddContact(std::vector<ParticleContact>& Contacts, int StartIndex) const = 0;

protected:

    float GetCurLength() const;
};


class ParticleCableConstraint : public ParticleConstraint
{

public:

    float m_maxLength = 0.0f;
    float m_restitution = 0.0f;

    virtual int AddContact(std::vector<ParticleContact>& Contacts, int StartIndex) const;
};


class GroundContacts : public ParticleContactGenerator
{

public:

    void Init(std::vector<Particle*>* pParticles);

    virtual int AddContact(std::vector<ParticleContact>& Contacts, int StartIndex) const;

private:

    std::vector<Particle*>* m_pParticles = NULL;
};


}