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

namespace OgldevPhysics
{
class Particle;

class ForceGenerator {
public:
    virtual void UpdateForce(Particle* pParticle, float dt) = 0;
};


class ForceRegistry {
public:

    void Add(Particle* pParticle, ForceGenerator* pForceGenerator);

    void Remove(Particle* pParticle, ForceGenerator* pForceGenerator);

    void Clear();

    void Update(float dt);

protected:

    struct ForceEntry {
        ForceEntry(Particle* pParticleIn, ForceGenerator* pForceGeneratorIn)
        {
            pParticle = pParticleIn;
            pForceGenerator = pForceGeneratorIn;
        }
        
        Particle* pParticle = NULL;
        ForceGenerator* pForceGenerator = NULL;
    };

    typedef std::vector<ForceEntry> Registry;
    Registry m_forceRegistry;
};

}
