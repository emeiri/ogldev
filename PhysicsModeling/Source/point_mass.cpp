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

#include "point_mass.h"

namespace Physics {

    void PointMass::Init(float Mass, const glm::vec3& Force, void* pTarget)
    {
        assert(Mass != 0.0f);

        m_mass = Mass;
        m_sumForces = Force;
        m_pTarget = pTarget;
    }


    void PointMass::Update(int DeltaTimeMillis, UpdateListener pUpdateListener)
{
    if (pUpdateListener) {
        (*pUpdateListener)(m_pTarget, m_centerOfMass);
    }

    assert(m_mass != 0.0f);

    m_linearAccel = m_sumForces / m_mass;

    float DeltaTime = (float)DeltaTimeMillis / 1000.0f;

    m_linearVelocity += m_linearVelocity * DeltaTime;

    m_centerOfMass += m_linearVelocity * DeltaTime;
}

}
