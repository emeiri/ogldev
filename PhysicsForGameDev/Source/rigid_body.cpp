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
                     const glm::vec3& CenterOfMass,
                     const glm::vec3& StartPos, 
                     const glm::vec3& ForceVec, 
                     const glm::vec3& ForcePoint,
                     void* pTarget)
{
    m_linear.Init(Mass, CenterOfMass, StartPos, ForceVec, pTarget);    

    ApplyForceAtPoint(ForceVec, ForcePoint);
}

void RigidBody::SetShapeBox(float Width, float Height, float Depth)
{
    m_shape = RIGID_BODY_SHAPE_BOX;

    float hh = Height * Height;
    float ww = Width * Width;
    float dd = Depth * Depth;
    float o12m = 1.0f / 12.0f * m_linear.GetMass();

    glm::mat3 Ibody = glm::mat3(
        o12m * (hh + dd), 0,                0,
        0,                o12m * (ww + dd), 0,
        0,                0,                o12m * (ww + hh)
    );

    m_inertiaBody = Ibody;
    m_inertiaBodyInv = glm::inverse(m_inertiaBody);
}


void RigidBody::ApplyForceAtPoint(const glm::vec3& F, const glm::vec3& ForcePoint)
{
    glm::vec3 r = ForcePoint - m_linear.GetCenterOfMass();
    m_torqueAccum += glm::cross(r, F);
}

void GLMPrintQuat(const glm::quat& q) 
{
    printf("quat = (%f, %f, %f : %f)\n", q.x, q.y, q.z, q.w);
}


void RigidBody::Update(float dt, UpdateListener pUpdateListener)
{
    if (m_shape == RIGID_BODY_SHAPE_NONE) {
        printf("Rigid body shape not initialized\n");
        assert(0);
    }

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
        pUpdateListener(m_linear.GetTarget(), m_linear.GetPos(), m_orientation);
    }
}


void RigidBody::UpdateInertiaWorldInv() 
{
    glm::mat3 R = glm::toMat3(m_orientation);
    m_inertiaWorldInv = R * m_inertiaBodyInv * glm::transpose(R);
}

}

