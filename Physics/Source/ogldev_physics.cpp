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


#include "ogldev_physics.h"

namespace OgldevPhysics
{

void PhysicsSystem::Init(uint NumObjects, uint MaxContacts, uint Iterations)
{
    m_particles.resize(NumObjects);
    m_numParticles = 0;

    m_fireworks.resize(NumObjects);
    m_numFireworks = 0;

    InitFireworksConfig();

    m_resolver.Init(Iterations);

    m_contacts.resize(MaxContacts);

    m_contactGenerators.resize(MaxContacts);

    m_calcIters = (Iterations == 0);
}


Particle* PhysicsSystem::AllocParticle()
{
    if (m_numParticles == m_particles.size()) {
        printf("%s:%d - exceeded max number of particles\n", __FILE__, __LINE__);
        exit(1);
    }

    Particle* ret = &m_particles[m_numParticles];
    m_numParticles++;

    return ret;
}


Firework* PhysicsSystem::AllocFirework()
{
    if (m_numFireworks == m_fireworks.size()) {
        printf("%s:%d - exceeded max number of fireworks\n", __FILE__, __LINE__);
        exit(1);
    }

    Firework* ret = &m_fireworks[m_numFireworks];
    m_numFireworks++;

    return ret;
}


void PhysicsSystem::Update(long long DeltaTimeMillis)
{
    assert(DeltaTimeMillis >= 0.0f);

    float dt = (float)DeltaTimeMillis / 1000.0f;

    m_forceRegistry.Update(dt);

    ParticleUpdate(dt);

    FireworkUpdate(dt);

    return;
    uint UsedContacts = GenerateContacts();

    if (UsedContacts) {
        if (m_calcIters) {
            m_resolver.SetIterations(UsedContacts * 2);
        }

        m_resolver.ResolveContacts(m_contacts, UsedContacts, dt);
    }
}


void PhysicsSystem::ParticleUpdate(float dt)
{
    for (uint i = 0; i < m_numParticles; i++) {
        m_particles[i].Integrate(dt);
    }
}


void PhysicsSystem::FireworkUpdate(float dt)
{
 //   printf("Update fireworks dt %f\n", dt);

    bool Finished = true;
    for (uint i = 0; i < m_numFireworks; i++) {
        OgldevPhysics::Firework& firework = m_fireworks[i];
        int Type = firework.GetType();
        //    printf("%d type %d\n", i, Type);
        if (Type > 0) {
            Finished = false;
            if (firework.Update(dt)) {
                //  printf("remove\n");
                OgldevPhysics::FireworkConfig& Config = m_fireworkConfigs[Type - 1];
                firework.SetType(0);

                for (int j = 0; j < Config.m_payloads.size(); j++) {
                    //     printf("payload %d\n", j);
                    Create(Config.m_payloads[j].m_type, Config.m_payloads[j].m_count, &firework);
                }
                //   printf("---\n");
            }
            else {
                //        printf("dont remove\n");
            }
        }
    }
}


void PhysicsSystem::StartFrame()
{
    for (uint i = 0; i < m_numParticles; i++) {
        m_particles[i].ClearAccum(); // Also called from Particle::Integrate !!!
    }
}


void PhysicsSystem::AddContact(ParticleContactGenerator* pContact)
{
    if (m_numContacts == m_contactGenerators.size()) {
        printf("Out of contact generators\n");
        exit(1);
    }

    m_contactGenerators[m_numContacts] = pContact;
    m_numContacts++;
}


uint PhysicsSystem::GenerateContacts()
{
    uint Limit = (uint)m_contacts.size();

    int NextContactIndex = 0;

    for (size_t i = 0; i < m_contactGenerators.size(); i++) {
        ParticleContact& NextContact = m_contacts[NextContactIndex];
        uint UsedContacts = m_contactGenerators[i]->AddContact(NextContact, Limit);
        Limit -= UsedContacts;
        NextContactIndex += Limit;

        if (Limit <= 0) {
            printf("Warning! Out of contacts\n");
            break;
        }
    }

    uint NumContactsUsed = (uint)m_contacts.size() - Limit;

    return NumContactsUsed;
}


}