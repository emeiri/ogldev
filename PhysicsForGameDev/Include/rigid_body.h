/*

        Copyright 2026 Etay Meiri

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
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "point_mass.h"

namespace Physics {

class RigidBody {
public:

    void Init(float Mass,
              const glm::vec3& StartPos,
              const glm::vec3& ForceVec,
              const glm::vec3& ForcePoint,
              void* pTarget);
   // void SetForce(const glm::vec3& F);

    void ApplyForceAtPoint(const glm::vec3& F, const glm::vec3& worldPoint);

    void Update(float dt, UpdateListener pUpdateListener);

private:
    void UpdateInertiaWorldInv();

    PointMass m_linear;        // handles mass, position, velocity, forces

    glm::quat m_orientation;
    glm::vec3 m_angularVelocity;
    glm::vec3 m_torqueAccum;

    glm::mat3 m_inertiaBody;
    glm::mat3 m_inertiaBodyInv;
    glm::mat3 m_inertiaWorldInv;
};
}
