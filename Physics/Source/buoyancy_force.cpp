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

#include "particle.h"
#include "buoyancy_force_generator.h"


namespace OgldevPhysics {

BuoyancyForceGenerator::BuoyancyForceGenerator(float MaxDepth, float Volume, float WaterHeight, float LiquidDensity)
{
    Init(MaxDepth, Volume, WaterHeight, LiquidDensity);
}


void BuoyancyForceGenerator::Init(float MaxDepth, float Volume, float WaterHeight, float LiquidDensity)
{
    m_maxDepth = MaxDepth;
    m_volume = Volume;
    m_waterHeight = WaterHeight;
    m_liquidDensity = LiquidDensity;
}

  
void BuoyancyForceGenerator::UpdateForce(Particle* pParticle, float dt)
{
    float Depth = pParticle->GetPosition().y;

    if (Depth >= m_waterHeight + m_maxDepth) {
        // we are out of the water
    } else {
        Vector3f Force(0.0f, 0.0f, 0.0f);

        if (Depth <= m_waterHeight - m_maxDepth) {
            // we are in maximum depth
            Force.y = m_liquidDensity * m_volume;
        } else {
            // we are partially submerged
            Force.y = m_liquidDensity * m_volume * (Depth - m_maxDepth - m_waterHeight) / 2.0f * m_maxDepth;
        }

        pParticle->AddForce(Force);
    }
}

}