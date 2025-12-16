/*

        Copyright 2025 Etay Meiri

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

#include <stdio.h>

#include "physics_system.h"

namespace Physics {

void System::Init(int NumPointMasses, UpdateListener pUpdateListener)
{
    m_pUpdateListener = pUpdateListener;
    m_pointMasses.resize(NumPointMasses);
}


void System::Update(int DeltaTimeMillis)
{
    float DeltaTime = DeltaTimeMillis / 1000.f;

    UpdateInternal(DeltaTime);
    HandleCollisions();
}


void System::UpdateInternal(float DeltaTime)
{    
    for (int i = 0; i < m_numActivePointMasses; i++) {
        m_pointMasses[i].Update(DeltaTime, m_pUpdateListener);
    }
}


void System::HandleCollisions()
{
    for (int i = 0; i < m_numActivePointMasses; i++) {
        for (int j = i + 1; j < m_numActivePointMasses; j++) {
            PointMass& OtherParticle = m_pointMasses[j];
            m_pointMasses[i].HandleCollision(OtherParticle);
        }
    }
}


PointMass* System::AllocPointMass()
{
    if (m_numActivePointMasses == (int)m_pointMasses.size()) {
        printf("Out of point masses\n");
        assert(0);
    }

    PointMass* pm = &m_pointMasses[m_numActivePointMasses];

    m_numActivePointMasses++;

    return pm;
}

 
}
