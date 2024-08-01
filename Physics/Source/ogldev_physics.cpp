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

void PhysicsSystem::Init(uint NumParticles)
{
    m_particles.resize(NumParticles);
    m_numParticles = 0;
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


void PhysicsSystem::Update(long long DeltaTimeMillis)
{
    assert(DeltaTimeMillis > 0.0f);

    float dt = (float)DeltaTimeMillis / 1000.0f;

    for (uint i = 0; i < m_numParticles; i++) {
        m_particles[i].Integrate(dt);
    }
}


}