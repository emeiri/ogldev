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

    RigidBody() {}

    void Init(float Mass,
              const glm::vec3& CenterOfMassLocal,
              const glm::vec3& StartPosWorld,
              const glm::vec3& ForceVecWorld,
              const glm::vec3& ForcePointLocal,
              void* pTarget);

    void SetShapeBox(float Width, float Height, float Depth);
       
    void Update(float dt, UpdateListener pUpdateListener);

    void ApplyForceAtPoint(const glm::vec3& ForceWorld, const glm::vec3& PointLocal);

    void AddForce(const glm::vec3& Force) { m_linear.AddForce(Force); }

    void ResetForces() { m_linear.ResetForces(); m_torqueAccum = glm::vec3(0); }

    void CalcCollisionReactions(RigidBody& OtherBody);

    PointMass& GetLinear() { return m_linear; }

    const glm::vec3& GetAngularVelocity() const { return m_angularVelocity; }

    void SetAngularVelocity(const glm::vec3& AngularVelocity) { m_angularVelocity = AngularVelocity; }

    void ReverseTorque() { m_torqueAccum *= -1.0f; }

    void SetOrientation(const glm::quat& Orientation) { m_orientation = Orientation; }

  //  const glm::vec3& GetInertiaLocal() const { return m_inertiaLocal; }

private:    

    void UpdateInertiaWorldInv();
    glm::vec3 LocalToWorld(const glm::vec3& p) const;

	// Handles the translational motion of the rigid body
    PointMass m_linear;

    // Handles the rotational motion
    glm::quat m_orientation = glm::quat(1, 0, 0, 0);
    glm::vec3 m_angularVelocity = glm::vec3(0.0f);
    glm::vec3 m_torqueAccum = glm::vec3(0.0f);
    
    glm::mat3 m_inertiaLocalInv = glm::mat3(0.0f);
    glm::mat3 m_inertiaWorldInv = glm::mat3(0.0f);
    glm::vec3 m_inertiaLocal = glm::vec3(0.0f);

    RIGID_BODY_SHAPE m_shape = RIGID_BODY_SHAPE_NONE;
};


void HandleOverlappingBodies(float DeltaTime, RigidBody& Body1, RigidBody& Body2);

}
