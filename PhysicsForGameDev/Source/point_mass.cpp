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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "point_mass.h"
#include "ogldev_util.h"

#define GLM_PRINT_VEC3(s, v) printf("%s (%f,%f,%f)\n", s, v.x, v.y, v.z)

namespace Physics {

void PointMass::Init(float Mass, const glm::vec3& CenterOfMass, const glm::vec3& StartPos, const glm::vec3& Force, void* pTarget)
{
    assert(Mass != 0.0f);

    m_pos = StartPos;

    m_mass = Mass;
    m_centerOfMass = CenterOfMass;
    m_sumForces = Force;
    m_pTarget = pTarget;

    m_linearAccel = glm::vec3(0.0f);
    m_linearVelocity = glm::vec3(0.0f);
}


void PointMass::Update(float DeltaTime, UpdateListener pUpdateListener)
{
  //  GLM_PRINT_VEC3("Sum forces ", m_sumForces);
    m_linearAccel = m_sumForces / m_mass;
 //   GLM_PRINT_VEC3("Linear accel ", m_linearAccel);
    //m_pos += (m_linearVelocity * DeltaTime) + (0.5f * m_linearAccel * DeltaTime * DeltaTime);
    m_pos += m_linearVelocity * DeltaTime;
  //  printf("PointMass %p position delta time %f ", this, DeltaTime); GLM_PRINT_VEC3("", m_pos);
    m_linearVelocity += m_linearAccel * DeltaTime;
  //  GLM_PRINT_VEC3("Linear velocity ", m_linearVelocity);    

    if (pUpdateListener) {
        glm::quat t(0.0f, 0.0f, 0.0f, 0.0f);
        (*pUpdateListener)(m_pTarget, m_pos, t);
    }

    m_sumForces = glm::vec3(0.0f);   
}

void PointMass::SetBoundingRadius(float r)
{
    if (r < 0.0f) {
        OGLDEV_ERROR0("Bounding radius must be >= 0");
        assert(0);
    }

    m_boundingRadius = r;
}


void PointMass::HandleCollision(PointMass& OtherParticle)
{
    if ((m_boundingRadius > 0.0f) && (OtherParticle.m_boundingRadius)) {
        bool CollisionOccured = CheckCollision(OtherParticle);

        if (CollisionOccured) {
            printf("Collision\n");

            float AvgCoeffRest = (m_coeffOfRest + OtherParticle.m_coeffOfRest) * 0.5f;

            if (AvgCoeffRest == 0.0f) {
                HandleCollisionInelastic(OtherParticle);
            } else {
                HandleCollisionElastic(OtherParticle, AvgCoeffRest);
            }
        }
    }
}


void PointMass::HandleCollisionElastic(Physics::PointMass& OtherParticle, float AvgCoeffRest)
{
    glm::vec3 CollisionNormal = m_pos - OtherParticle.m_pos;

    float Distance = glm::length(CollisionNormal);

    if (Distance > 0.0) {
        CollisionNormal /= Distance;    // Normalize

        // 4. Check if objects are moving toward each other
        glm::vec3 RelVelocity = m_linearVelocity - OtherParticle.m_linearVelocity;
        float RelNormalVel = glm::dot(RelVelocity, CollisionNormal);

        if (RelNormalVel <= 0.0f) { // Skip if they are already separating

            float Velocity1 = glm::dot(m_linearVelocity, CollisionNormal);
            float Velocity2 = glm::dot(OtherParticle.m_linearVelocity, CollisionNormal);

            float SumOfMass = m_mass + OtherParticle.m_mass;
            float AvgCoeffRestPlusOne = (1.0f + AvgCoeffRest);

            float Nom = ((m_mass - AvgCoeffRest * OtherParticle.m_mass) * Velocity1) +
                (AvgCoeffRestPlusOne * OtherParticle.m_mass * Velocity2);

            float FinalVelocity1 = Nom / SumOfMass;

            Nom = ((OtherParticle.m_mass - AvgCoeffRest * m_mass) * Velocity2) +
                (AvgCoeffRestPlusOne * m_mass * Velocity1);

            float FinalVelocity2 = Nom / SumOfMass;

            m_linearVelocity += (FinalVelocity1 - Velocity1) * CollisionNormal;
            OtherParticle.m_linearVelocity += (FinalVelocity2 - Velocity2) * CollisionNormal;
        }
    }
}


void PointMass::HandleCollisionInelastic(Physics::PointMass& OtherParticle)
{
    glm::vec3 TotalMomentum = m_mass * m_linearVelocity +
        OtherParticle.m_mass * OtherParticle.m_linearVelocity;

    glm::vec3 SharedVelocity = TotalMomentum / (m_mass + OtherParticle.m_mass);

    m_linearVelocity = SharedVelocity;
    OtherParticle.m_linearVelocity = SharedVelocity;
}


bool PointMass::CheckCollision(const PointMass& OtherParticle) const
{
    COLLISION_STATUS Status = GetCollisionStatus(OtherParticle);
 
    bool CollisionOccured = Status != COLLISION_STATUS_NONE;

    return CollisionOccured;
}


COLLISION_STATUS PointMass::GetCollisionStatus(const PointMass& OtherParticle) const
{
    float DistSquared = glm::length2(m_pos - OtherParticle.m_pos);
    float MinDistanceSquared = m_boundingRadius + OtherParticle.m_boundingRadius;
    MinDistanceSquared *= MinDistanceSquared;

    COLLISION_STATUS Status = COLLISION_STATUS_NONE;

    const float Epsilon = 1e-5f;

    if (DistSquared < MinDistanceSquared) {
        Status = COLLISION_STATUS_OVERLAPPING;
    } else if (fabsf(DistSquared - MinDistanceSquared) < Epsilon) {
        Status = COLLISION_STATUS_TOUCHING;
    }

    return Status;
}

}