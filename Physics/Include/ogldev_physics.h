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

namespace OgldevPhysics
{

#define DEFAULT_NUM_PARTICLES 1000

class PhysicsSystem {

public:
    PhysicsSystem() {}

    ~PhysicsSystem() {}

    void Init(uint NumParticles = DEFAULT_NUM_PARTICLES);

    Particle* AllocParticle();

    void Update(long long DeltaTimeMillis);

private:

    std::vector<Particle> m_particles;
    uint m_numParticles = 0;
};

}