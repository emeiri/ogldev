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

void PhysicsSystem::Init(uint NumObjects)
{
    m_particles.resize(NumObjects);
    m_numParticles = 0;

    m_fireworks.resize(NumObjects);
    m_numFirework = 0;

    InitFireworksConfig();

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
    if (m_numFirework == m_fireworks.size()) {
        printf("%s:%d - exceeded max number of fireworks\n", __FILE__, __LINE__);
        exit(1);
    }

    Firework* ret = &m_fireworks[m_numFirework];
    m_numFirework++;

    return ret;
}


void PhysicsSystem::Update(long long DeltaTimeMillis)
{
    assert(DeltaTimeMillis >= 0.0f);

    float dt = (float)DeltaTimeMillis / 1000.0f;

    ParticleUpdate(dt);

    FireworkUpdate(dt);
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
    for (int i = 0; i < m_fireworks.size(); i++) {
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


}