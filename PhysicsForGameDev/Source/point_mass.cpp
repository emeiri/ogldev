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

namespace Physics {

void PointMass::Init(float Mass, const glm::vec3& StartPos, const glm::vec3& Force, void* pTarget)
{
    assert(Mass != 0.0f);

    m_mass = Mass;
    m_centerOfMass = StartPos;
    m_sumForces = Force;
    m_pTarget = pTarget;

    m_linearAccel = glm::vec3(0.0f);
    m_linearVelocity = glm::vec3(0.0f);
}


void PointMass::Update(float DeltaTime, UpdateListener pUpdateListener)
{    
    m_linearAccel = m_sumForces / m_mass;

    m_linearVelocity += m_linearAccel * DeltaTime;

    m_centerOfMass += m_linearVelocity * DeltaTime;

    if (pUpdateListener) {
        (*pUpdateListener)(m_pTarget, m_centerOfMass);
    }
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
                glm::vec3 totalMomentum = m_mass * m_linearVelocity +
                                          OtherParticle.m_mass * OtherParticle.m_linearVelocity;

                glm::vec3 sharedVelocity = totalMomentum / (m_mass + OtherParticle.m_mass);

                m_linearVelocity = sharedVelocity;
                OtherParticle.m_linearVelocity = sharedVelocity;
            } else {
                glm::vec3 CollisionNormal = glm::normalize(m_centerOfMass - OtherParticle.m_centerOfMass);

                float Velocity1 = glm::dot(m_linearVelocity, CollisionNormal);
                float Velocity2 = glm::dot(OtherParticle.m_linearVelocity, CollisionNormal);

                float SumOfMass = m_mass + OtherParticle.m_mass;
                float AvgCoeffRestPlusOne = (1.0f + AvgCoeffRest);

                float Nom = ((m_mass - AvgCoeffRest * OtherParticle.m_mass) * Velocity1) +
                               (AvgCoeffRestPlusOne * OtherParticle.m_mass  * Velocity2);

                float FinalVelocity1 = Nom / SumOfMass;

                Nom = ((OtherParticle.m_mass - AvgCoeffRest * m_mass) * Velocity2) +
                                       (AvgCoeffRestPlusOne * m_mass  * Velocity1);

                float FinalVelocity2 = Nom / SumOfMass;

                m_linearVelocity               += (FinalVelocity1 - Velocity1) * CollisionNormal;
                OtherParticle.m_linearVelocity += (FinalVelocity2 - Velocity2) * CollisionNormal;

                m_linearAccel               = m_linearVelocity / DeltaTime;
                OtherParticle.m_linearAccel = OtherParticle.m_linearVelocity / DeltaTime;

                m_sumForces               = m_linearAccel * m_mass;
                OtherParticle.m_sumForces = OtherParticle.m_linearAccel * OtherParticle.m_mass;
            }
        }
    }    
}


bool PointMass::CheckCollision(const PointMass& OtherParticle) const
{
    float DistSquared = glm::length2(m_centerOfMass - OtherParticle.m_centerOfMass);

    float MinDistanceSquared = m_boundingRadius + OtherParticle.m_boundingRadius;
    MinDistanceSquared *= MinDistanceSquared;

    bool CollisionOccured = DistSquared <= MinDistanceSquared;

    return CollisionOccured;
}

}
