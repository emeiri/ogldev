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
    m_centerOfMassLocal = CenterOfMassLocal;
    m_orientation = StartOrientation;
	m_angularVelocity = glm::vec3(0);
	m_torqueAccum = glm::vec3(0);

    m_linear.Init(Mass, StartPosWorld, ForceVecWorld, pTarget);

    ApplyForceAtPoint(ForceVecWorld, ForcePointLocal);
}


void RigidBody::ApplyForceAtPoint(const glm::vec3& ForceWorld,
                                  const glm::vec3& PointLocal)
{
    glm::vec3 PointWorld = LocalToWorld(PointLocal);

    glm::vec3 CenterOfMassWorld = LocalToWorld(m_centerOfMassLocal);

    // Lever arm in world space
    glm::vec3 RadiusWorld = PointWorld - CenterOfMassWorld;

    // Torque in world space
    m_torqueAccum += glm::cross(RadiusWorld, ForceWorld);
   // printf("%p Applying force at point: ", this); GLM_PRINT_VEC3("", m_torqueAccum);
}


glm::vec3 RigidBody::LocalToWorld(const glm::vec3& LocalPos) const
{
    glm::mat3 R = glm::toMat3(m_orientation);
    return R * LocalPos + m_linear.GetPos();
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
    glm::quat qdot = 0.5f * wq * m_orientation;

    m_orientation += qdot * dt;
    m_orientation = glm::normalize(m_orientation);

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

    // 2. Transform the center of mass from local to world space
    glm::vec3 CoMA_World = LocalToWorld(m_centerOfMassLocal);
    glm::vec3 CoMB_World = OtherBody.LocalToWorld(OtherBody.m_centerOfMassLocal);

    // 3. The radius from the center of mass to the contact point in world space
    glm::vec3 rA = contactPoint - CoMA_World;
    glm::vec3 rB = contactPoint - CoMB_World;

    // 4. Velocities at contact
    glm::vec3 vA = m_linear.GetLinearVelocity() + glm::cross(m_angularVelocity, rA);
    glm::vec3 vB = OtherBody.m_linear.GetLinearVelocity() + glm::cross(OtherBody.m_angularVelocity, rB);
    glm::vec3 relVel = vA - vB;
    float relVelAlongNormal = glm::dot(relVel, normal);

    // 5. Early exit if objects are moving apart
    if (relVelAlongNormal > 0) {
        return;
    }

    // 6. Inverse mass and inertia
    float invMassA = 1.0f / m_linear.GetMass();
    float invMassB = 1.0f / OtherBody.m_linear.GetMass();
    glm::mat3 invInertiaA = m_inertiaWorldInv;
    glm::mat3 invInertiaB = OtherBody.m_inertiaWorldInv;

    GLM_PRINT_VEC3("Normal ", glm::cross(rA, normal));

    float denom = invMassA + invMassB + glm::dot(normal,
            glm::cross(invInertiaA * glm::cross(rA, normal), rA) +
            glm::cross(invInertiaB * glm::cross(rB, normal), rB));

    // 7. Average restitution
    float restitution = 0.5f * (m_linear.GetCoeffOfRest() + OtherBody.m_linear.GetCoeffOfRest());

    // 8. Impulse scalar
    float j = -(1.0f + restitution) * relVelAlongNormal / denom;

    // 9. Impulse scalar to vector
    glm::vec3 impulse = j * normal;

    // 10. Apply impulse
    m_linear.SetLinearVelocity(m_linear.GetLinearVelocity() + (impulse * invMassA));
    m_angularVelocity += invInertiaA * glm::cross(rA, impulse);

    // 11. And on the other one...
    OtherBody.m_linear.SetLinearVelocity(OtherBody.m_linear.GetLinearVelocity() - (impulse * invMassB));
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
    glm::vec3 DeltaNormalized = delta / dist;
    if (dist < minDist && dist > 0.0f) {
        glm::vec3 correction = (minDist - dist) * DeltaNormalized * 0.5f;
        glm::vec3 NewPos1 = Body1.GetLinear().GetPos() + correction;
        Body1.GetLinear().SetPos(NewPos1);
        glm::vec3 NewPos2 = Body2.GetLinear().GetPos() - correction;
        Body2.GetLinear().SetPos(NewPos2);
    }
}


void HandleOverlappingBodies(float DeltaTime, RigidBody& Body1, RigidBody& Body2)
{
    // 1. Update velocities based on collision
    Body1.CalcCollisionReactions(Body2);

    // 2. Move objects apart
    ResolvePenetration(Body1, Body2);
}


}

