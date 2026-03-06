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

#include "ogldev_math_3d.h"
#include "rigid_body.h"

namespace Physics {

void RigidBody::Init(float Mass,
                     const glm::vec3& CenterOfMassLocal,
                     const glm::vec3& StartPosWorld,
                     const glm::vec3& ForceVecWorld,
                     const glm::vec3& ForcePointLocal,
                     void* pTarget)
{
    m_orientation = glm::quat(1, 0, 0, 0);
	m_angularVelocity = glm::vec3(0);
	m_torqueAccum = glm::vec3(0);

    m_linear.Init(Mass, CenterOfMassLocal, StartPosWorld, ForceVecWorld, pTarget);

    ApplyForceAtPoint(ForceVecWorld, ForcePointLocal);
}


void RigidBody::ApplyForceAtPoint(const glm::vec3& ForceWorld,
                                  const glm::vec3& PointLocal)
{
    glm::vec3 PointWorld = LocalToWorld(PointLocal);

    glm::vec3 CenterOfMassWorld = LocalToWorld(m_linear.GetCenterOfMass());

    // Lever arm in world space
    glm::vec3 Radius = PointWorld - CenterOfMassWorld;

    // Torque in world space
    m_torqueAccum += glm::cross(Radius, ForceWorld);
}


glm::vec3 RigidBody::LocalToWorld(const glm::vec3& p) const
{
    glm::mat3 R = glm::toMat3(m_orientation);
    return R * p + m_linear.GetPos();
}


void RigidBody::SetShapeBox(float Width, float Height, float Depth)
{
    m_shape = RIGID_BODY_SHAPE_BOX;

    float hh = Height * Height;
    float ww = Width * Width;
    float dd = Depth * Depth;
    float MassDiv12 = 1.0f / 12.0f * m_linear.GetMass();

    glm::mat3 InertiaLocal = glm::mat3(
        MassDiv12 * (hh + dd), 0, 0,
        0, MassDiv12 * (ww + dd), 0,
        0, 0, MassDiv12 * (ww + hh)
    );

    m_inertiaLocalInv = glm::inverse(InertiaLocal);
}


void RigidBody::Update(float dt, UpdateListener pUpdateListener)
{
    if (m_shape == RIGID_BODY_SHAPE_NONE) {
        printf("Rigid body shape not initialized\n");
        assert(0);
    }

    m_linear.Update(dt, NULL);

    UpdateInertiaWorldInv();

    glm::vec3 AngularAccel = m_inertiaWorldInv * m_torqueAccum;
    m_angularVelocity += AngularAccel * dt;

    glm::quat wq(0, m_angularVelocity.x, m_angularVelocity.y, m_angularVelocity.z);
    glm::quat qdot = 0.5f * (m_orientation * wq);

    m_orientation += qdot * dt;
    m_orientation = glm::normalize(m_orientation);

    if (dt > 0.0f) {
        m_torqueAccum = glm::vec3(0);
    }

    if (pUpdateListener) {
        pUpdateListener(m_linear.GetTarget(), m_linear.GetPos(), m_orientation);
    }
}


void RigidBody::UpdateInertiaWorldInv()
{
    glm::mat3 R = glm::toMat3(m_orientation);
    m_inertiaWorldInv = R * m_inertiaLocalInv * glm::transpose(R);
}


void RigidBody::ResetSumForces()
{
	m_linear.ResetSumForces();
	m_torqueAccum = glm::vec3(0);
}


}