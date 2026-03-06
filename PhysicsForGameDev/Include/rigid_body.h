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

enum RIGID_BODY_SHAPE {
    RIGID_BODY_SHAPE_NONE,
    RIGID_BODY_SHAPE_BOX
};

class RigidBody {
public:

    void Init(float Mass,
              const glm::vec3& CenterOfMass,
              const glm::vec3& StartPos,
              const glm::vec3& ForceVec,
              const glm::vec3& ForcePoint,
              void* pTarget);

    void SetShapeBox(float Width, float Height, float Depth);
       
    void Update(float dt, UpdateListener pUpdateListener);

    void ApplyForceAtPoint(const glm::vec3& Force, const glm::vec3& AtPoint);

    void ResetSumForces();

    void AddForce(const glm::vec3& Force) { m_linear.AddForce(Force); }

private:    

    void UpdateInertiaWorldInv();
    glm::vec3 LocalToWorld(const glm::vec3& p) const;

    PointMass m_linear;        // handles mass, position, velocity, forces

    glm::quat m_orientation = glm::quat(1, 0, 0, 0);
    glm::vec3 m_angularVelocity;
    glm::vec3 m_torqueAccum;

    glm::mat3 m_inertiaLocalInv;
    glm::mat3 m_inertiaWorldInv;

    RIGID_BODY_SHAPE m_shape = RIGID_BODY_SHAPE_NONE;
};
}
