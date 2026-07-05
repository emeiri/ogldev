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

#include "ogldev_math_3d.h"

namespace Physics {

enum COLLISION_STATUS {
    COLLISION_STATUS_NONE,
    COLLISION_STATUS_TOUCHING,
    COLLISION_STATUS_OVERLAPPING
};

typedef void (*UpdateListener)(const void* pObject, const glm::vec3& Pos, const glm::quat& Orientation);

class PointMass {

public:

    void Init(float Mass, const glm::vec3& StartPos, const glm::vec3& Force, void* pTarget);

    // DeltaTime - 1.0f is 1 second.
    void Update(float DeltaTime, UpdateListener pUpdateListener);

    void ResetForces() { m_sumForces = glm::vec3(0.0f); }

    const glm::vec3 GetLinearVelocity() const { return m_linearVelocity; }

    void SetLinearVelocity(const glm::vec3& Vel) { m_linearVelocity = Vel; }

    void SetBoundingRadius(float r);

    void HandleCollision(PointMass& OtherParticle);

    void HandleCollisionElastic(Physics::PointMass& OtherParticle, float AvgCoeffRest);

    void AddForce(const glm::vec3& Force) { 
   //     printf("%p ", this); GLM_PRINT_VEC3("Adding force ", Force);
        m_sumForces += Force; 
    //    GLM_PRINT_VEC3("Sum forces ", m_sumForces);
    }

    const glm::vec3& GetPos() const { return m_pos; }

    void SetPos(const glm::vec3& Pos) { m_pos = Pos; }

    const void* GetTarget() const { return m_pTarget; }

    float GetMass() const { return m_mass; }

    float GetCoeffOfRest() const { return m_coeffOfRest; }

    float GetBoundingRadius() const { return m_boundingRadius; }

    COLLISION_STATUS GetCollisionStatus(const PointMass& OtherParticle) const;

private:

    bool CheckCollision(const PointMass& OtherParticle) const;    
    void HandleCollisionInelastic(Physics::PointMass& OtherParticle);

    glm::vec3 m_pos = glm::vec3(0.0f);

    // Physics stuff    
    glm::vec3 m_linearVelocity = glm::vec3(0.0f);
    glm::vec3 m_sumForces = glm::vec3(0.0f);
    glm::vec3 m_linearAccel = glm::vec3(0.0f);
    float m_mass = 0.0f;

    float m_boundingRadius = 0.0f;

    float m_coeffOfRest = 1.0f;

    float m_damping = 1.0f;

    // Client interface
    const void* m_pTarget = NULL;
};
}