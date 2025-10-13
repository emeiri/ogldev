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

#include <assert.h>
#include <vector>

#include "ogldev_types.h"
#include "particle.h"
#include "firework.h"
#include "gravity_force_generator.h"
#include "drag_force_generator.h"
#include "spring_force_generator.h"
#include "anchored_spring_force_generator.h"
#include "bungee_spring_force_generator.h"
#include "buoyancy_force_generator.h"
#include "fake_spring_force_generator.h"
#include "contact_resolver.h"

namespace OgldevPhysics
{

const static Vector3f GRAVITY = Vector3f(0.0f, -9.81f, 0.0f);

class PhysicsSystem {

public:
    PhysicsSystem() {}

    ~PhysicsSystem() {}

    void Init(uint NumParticles, uint MaxContacts, uint Iterations);

    Particle* AllocParticle();

    Firework* AllocFirework();

    void Update(long long DeltaTimeMillis);

    ForceRegistry& GetRegistry() { return m_forceRegistry; }    

    void StartFrame();

    void AddContactGenerator(ParticleContactGenerator* pContact);

private:

    void InitFireworksConfig();

    void Create(int Type, uint Count, Firework* pFirework);

    void ParticleUpdate(float dt);
    void FireworkUpdate(float dt);

    uint GenerateContacts();

    std::vector<Particle> m_particles;
    std::vector<Firework> m_fireworks;
    std::vector<FireworkConfig> m_fireworkConfigs;
    std::vector<ParticleContactGenerator*> m_contactGenerators;
    std::vector<ParticleContact> m_contacts;

    ForceRegistry m_forceRegistry;
    ParticleContactResolver m_resolver;

    uint m_numParticles = 0;
    uint m_numFireworks = 0;
    uint m_nextFirework = 0; 
    uint m_numContactGenerators = 0;
    bool m_calcIters = false;   
};

}