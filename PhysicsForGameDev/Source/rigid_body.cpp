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

#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "rigid_body.h"

namespace Physics {

void RigidBody::Init(float Mass, 
                     const glm::vec3& StartPos, 
                     const glm::vec3& ForceVec, 
                     const glm::vec3& ForcePoint,
                     void* pTarget)
{
    m_linear.Init(Mass, StartPos, ForceVec, pTarget);

    // Set inertia tensor for a box (example)
    float w = 1.0f, h = 1.0f, d = 0.5f;
    float m = 5.0f;

    glm::mat3 Ibody = glm::mat3(
        (1.0f / 12.0f) * m * (h * h + d * d), 0, 0,
        0, (1.0f / 12.0f) * m * (w * w + d * d), 0,
        0, 0, (1.0f / 12.0f) * m * (w * w + h * h)
    );

    m_inertiaBody = Ibody;
    m_inertiaBodyInv = glm::inverse(m_inertiaBody);

    m_orientation = glm::quat(1, 0, 0, 0);

    ApplyForceAtPoint(ForceVec, ForcePoint);
}


void RigidBody::ApplyForceAtPoint(const glm::vec3& F, const glm::vec3& worldPoint)
{
    glm::vec3 r = worldPoint - m_linear.GetCenterOfMass();
    m_torqueAccum += glm::cross(r, F);
}

void GLMPrintQuat(const glm::quat& q) 
{
    printf("quat = (%f, %f, %f : %f)\n", q.x, q.y, q.z, q.w);
}


void RigidBody::Update(float dt, UpdateListener pUpdateListener)
{
    m_linear.Update(dt, NULL);

    UpdateInertiaWorldInv();

    glm::vec3 alpha = m_inertiaWorldInv * m_torqueAccum;
    m_angularVelocity += alpha * dt;

    glm::quat wq(0, m_angularVelocity.x, m_angularVelocity.y, m_angularVelocity.z);
    glm::quat qdot = 0.5f * (m_orientation * wq);

    m_orientation += qdot * dt;
    m_orientation = glm::normalize(m_orientation);

    if (dt > 0.0f) {
        m_torqueAccum = glm::vec3(0);
    }

  //  GLMPrintQuat(m_orientation);

    if (pUpdateListener) {
        pUpdateListener(m_linear.GetTarget(), m_linear.GetCenterOfMass(), m_orientation);
    }
}


void RigidBody::UpdateInertiaWorldInv() 
{
    glm::mat3 R = glm::toMat3(m_orientation);
    m_inertiaWorldInv = R * m_inertiaBodyInv * glm::transpose(R);
}

}

