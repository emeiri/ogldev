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
                     const glm::quat& StartOrientation,
                     void* pTarget)
{
    m_orientation = StartOrientation;
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
    glm::vec3 RadiusWorld = PointWorld - CenterOfMassWorld;

    // Torque in world space
    m_torqueAccum += glm::cross(RadiusWorld, ForceWorld);
   // printf("%p Applying force at point: ", this); GLM_PRINT_VEC3("", m_torqueAccum);
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

    m_inertiaLocal = glm::vec3(MassDiv12 * (hh + dd), 
                               MassDiv12 * (ww + dd), 
                               MassDiv12 * (ww + hh));

    glm::mat3 InertiaLocalMat = glm::mat3(
        m_inertiaLocal[0], 0,                 0,
        0,                 m_inertiaLocal[1], 0,
        0,                 0,                 m_inertiaLocal[2]
    );    

    m_inertiaLocalInv = glm::inverse(InertiaLocalMat);
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

	// We reset the torque accumulator after the update. We cannot assume that the force 
    // is still in effect in the next frame, so we need to reset it. We also need to reset 
    // it after the first update, because the initial force is only applied once.
	// The check for dt > 0 is to avoid resetting the torque accumulator before the 
    // first update, which would cause the initial force to be ignored.
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


void RigidBody::CalcCollisionReactions(RigidBody& OtherBody)
{
    // 1. Compute contact point and normal (for spheres, use centers and radii)
    glm::vec3 normal = glm::normalize(m_linear.GetPos() - OtherBody.m_linear.GetPos());
    glm::vec3 contactPoint = m_linear.GetPos() - normal * m_linear.GetBoundingRadius();

    // 2. Relative positions from centers to contact
    glm::vec3 rA = contactPoint - m_linear.GetPos();
    glm::vec3 rB = contactPoint - OtherBody.m_linear.GetPos();

    // 3. Velocities at contact
    glm::vec3 vA = m_linear.GetLinearVelocity() + glm::cross(m_angularVelocity, rA);
    glm::vec3 vB = OtherBody.m_linear.GetLinearVelocity() + glm::cross(OtherBody.m_angularVelocity, rB);
    glm::vec3 relVel = vA - vB;
    float relVelAlongNormal = glm::dot(relVel, normal);

    // 4. Restitution
    float restitution = 0.5f * (m_linear.GetCoeffOfRest() + OtherBody.m_linear.GetCoeffOfRest());

    // 5. Inverse mass and inertia
    float invMassA = 1.0f / m_linear.GetMass();
    float invMassB = 1.0f / OtherBody.m_linear.GetMass();
    glm::mat3 invInertiaA = m_inertiaWorldInv;
    glm::mat3 invInertiaB = OtherBody.m_inertiaWorldInv;

    // 6. Impulse denominator
    float denom = invMassA + invMassB +
        glm::dot(normal,
            glm::cross(invInertiaA * glm::cross(rA, normal), rA) +
            glm::cross(invInertiaB * glm::cross(rB, normal), rB));

    // 7. Impulse scalar
    float j = -(1.0f + restitution) * relVelAlongNormal / denom;

    // 8. Apply impulse
    glm::vec3 impulse = j * normal;
    m_linear.AddForce(impulse);
    m_angularVelocity += invInertiaA * glm::cross(rA, impulse);

    OtherBody.m_linear.AddForce(-impulse);
    OtherBody.m_angularVelocity -= invInertiaB * glm::cross(rB, impulse);
}

static void ReverseBody(RigidBody& Body)
{
    glm::vec3 t = Body.GetAngularVelocity();
    t *= -1.0f;
    Body.SetAngularVelocity(t);

    t = Body.GetLinear().GetLinearVelocity();
    t *= -1.0f;
    Body.GetLinear().SetLinearVelocity(t);

    Body.ReverseTorque();
}


bool CloseToZero(float value, float epsilon = 1e-6f)
{
    return fabsf(value) < epsilon;
}


void HandleOverlappingBodies2(float DeltaTime, RigidBody& Body1, RigidBody& Body2)
{
    bool Done = false;
    COLLISION_STATUS cs = COLLISION_STATUS_OVERLAPPING;

    while (!Done && !CloseToZero(DeltaTime)) {

        switch (cs) {

        case COLLISION_STATUS_OVERLAPPING:
            printf("HandleOverlappingBodies: Overlapping delta time %f\n", DeltaTime);
            ReverseBody(Body1);
            ReverseBody(Body2);
            Body1.Update(DeltaTime, NULL);
            Body2.Update(DeltaTime, NULL);

            DeltaTime *= 0.5f;

            ReverseBody(Body1);
            ReverseBody(Body2);
            Body1.Update(DeltaTime, NULL);
            Body2.Update(DeltaTime, NULL);

            cs = Body1.GetLinear().GetCollisionStatus(Body2.GetLinear());
            break;

        case COLLISION_STATUS_TOUCHING:
            printf("HandleOverlappingBodies: touching\n");
            Body1.CalcCollisionReactions(Body2);
            Done = true;
            break;

        case COLLISION_STATUS_NONE:
            printf("HandleOverlappingBodies: none\n");
            Body1.Update(DeltaTime, NULL);
            Body2.Update(DeltaTime, NULL);
            Done = true;
            break;
        }
    }
}


void ResolvePenetration(RigidBody& Body1, RigidBody& Body2)
{
    glm::vec3 delta = Body1.GetLinear().GetPos() - Body2.GetLinear().GetPos();
    float dist = glm::length(delta);
    float minDist = Body1.GetLinear().GetBoundingRadius() + Body2.GetLinear().GetBoundingRadius();
    if (dist < minDist && dist > 0.0f) {
        glm::vec3 correction = (minDist - dist) * (delta / dist) * 0.5f;
        glm::vec3 NewPos1 = Body1.GetLinear().GetPos() + correction;
        Body1.GetLinear().SetPos(NewPos1);
        glm::vec3 NewPos2 = Body2.GetLinear().GetPos() - correction;
        Body2.GetLinear().SetPos(NewPos2);
    }
}


void HandleOverlappingBodies(float DeltaTime, RigidBody& Body1, RigidBody& Body2)
{
    COLLISION_STATUS cs = Body1.GetLinear().GetCollisionStatus(Body2.GetLinear());

    if (cs == COLLISION_STATUS_OVERLAPPING) {
        // 1. Move objects apart
        ResolvePenetration(Body1, Body2);

        // 2. Update velocities based on collision
        Body1.CalcCollisionReactions(Body2);
    } else if (cs == COLLISION_STATUS_TOUCHING) {
        Body1.CalcCollisionReactions(Body2);
    }

    // 3. Update both bodies for this time step
    Body1.Update(DeltaTime, NULL);
    Body2.Update(DeltaTime, NULL);
}


}

