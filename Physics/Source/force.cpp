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


#include "force_generator.h"

namespace OgldevPhysics {


void ForceRegistry::Add(Particle* pParticle, ForceGenerator* pForceGenerator)
{
    ForceEntry Entry(pParticle, pForceGenerator);
    m_forceRegistry.push_back(Entry);
}


void ForceRegistry::Update(float dt)
{
    for (Registry::iterator it = m_forceRegistry.begin(); it != m_forceRegistry.end(); it++) {
        it->pForceGenerator->UpdateForce(it->pParticle, dt);
    }
}
}