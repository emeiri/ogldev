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


#pragma once

#include <glm/glm.hpp>

namespace Physics {

typedef void (*UpdateListener)(void* pObject, const glm::vec3& Pos);

class PointMass {

public:

    void Init(float Mass, const glm::vec3& StartPos, const glm::vec3& Force, void* pTarget);

    void Update(int DeltaTimeMillis, UpdateListener pUpdateListener);

    void ResetSumForces() { m_sumForces = glm::vec3(0.0f); }

    glm::vec3 m_centerOfMass = glm::vec3(0.0f);
    glm::vec3 m_linearVelocity = glm::vec3(0.0f);
    glm::vec3 m_sumForces = glm::vec3(0.0f);
    void* m_pTarget = NULL;
    float m_mass = 0.0f;     

private:
    glm::vec3 m_linearAccel = glm::vec3(0.0f);
};
}