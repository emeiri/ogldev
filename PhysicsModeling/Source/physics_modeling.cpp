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


#include "physics_modeling.h"

namespace Physics {

void System::Init(int NumPointMasses, UpdateListener pUpdateListener)
{
    m_pUpdateListener = pUpdateListener;
    m_pointMasses.resize(NumPointMasses);
}


void System::Update(int DeltaTimeMillis)
{
    for (int i = 0 ; i < m_numPointMasses ; i++) {
        m_pointMasses[i].Update(DeltaTimeMillis, m_pUpdateListener);
    }
}

PointMass* System::AllocPointMass()
{
    if (m_numPointMasses == (int)m_pointMasses.size()) {
        printf("Out of point masses\n");
        assert(0);
    }

    PointMass* pm = &m_pointMasses[m_numPointMasses];

    m_numPointMasses++;

    return pm;
}
 
}
