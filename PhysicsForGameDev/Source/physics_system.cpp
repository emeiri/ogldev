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

#include <stdio.h>

#include "physics_system.h"

namespace Physics {

void System::Init(int NumPointMasses, int NumRigidBodies, UpdateListener pUpdateListener, const glm::vec3& GlobalForce)
{
    m_pUpdateListener = pUpdateListener;
    m_globalForce = GlobalForce;
    m_pointMasses.resize(NumPointMasses);
    m_rigidBodies.resize(NumRigidBodies);
}


void System::Update(int DeltaTimeMillis)
{
    float DeltaTime = DeltaTimeMillis / 1000.f;

    if (DeltaTime > 0.1f) {
        DeltaTime = 0.1f;
    }

    static float accumulator = 0.0f;
    accumulator += DeltaTime;

    //printf("DeltaTime = %f\n", DeltaTime);
    // 2. Consume accumulated time in fixed dt chunks
    const float FixedDT = 1.0f / 60.0f;

    while (accumulator >= 1.0f/60.0f) {

        ApplyGlobalForces();

        // Pass the FIXED dt (1/60), NOT the variable frameTime
        UpdateInternal(FixedDT);
        HandlePointMassCollisions();
        HandleRigidBodyCollisions(FixedDT);

        for (int i = 0; i < m_numActiveRigidBodies; i++) {
            m_rigidBodies[i].ResetForces();
        }
        ResetAllForces();

        accumulator -= FixedDT;       
    }
}


void System::ApplyGlobalForces()
{
    for (int i = 0; i < m_numActivePointMasses; i++) {
        m_pointMasses[i].AddForce(m_globalForce);
    }

    for (int i = 0; i < m_numActiveRigidBodies; i++) {
        m_rigidBodies[i].AddForce(m_globalForce);
    }
}


void System::ResetAllForces()
{
    for (int i = 0; i < m_numActivePointMasses; i++) {
        m_pointMasses[i].ResetForces();
    }

    for (int i = 0; i < m_numActiveRigidBodies; i++) {
        m_rigidBodies[i].ResetForces();
    }
}


void System::UpdateInternal(float DeltaTime)
{    
    UpdatePointMasses(DeltaTime);
    UpdateRigidBodies(DeltaTime);
}


void System::UpdatePointMasses(float DeltaTime)
{
    for (int i = 0; i < m_numActivePointMasses; i++) {
        m_pointMasses[i].Update(DeltaTime, m_pUpdateListener);
    }
}


void System::UpdateRigidBodies(float DeltaTime)
{
    for (int i = 0; i < m_numActiveRigidBodies; i++) {
        m_rigidBodies[i].Update(DeltaTime, m_pUpdateListener);
    }
}


void System::HandlePointMassCollisions()
{
    for (int i = 0; i < m_numActivePointMasses; i++) {
        for (int j = i + 1; j < m_numActivePointMasses; j++) {
            PointMass& OtherParticle = m_pointMasses[j];
            m_pointMasses[i].HandleCollision(OtherParticle);
        }
    }
}


void System::HandleRigidBodyCollisions(float DeltaTime)
{
    for (int i = 0; i < m_numActiveRigidBodies; i++) {
        RigidBody& CurBody = m_rigidBodies[i];
        for (int j = i + 1; j < m_numActiveRigidBodies; j++) {
            RigidBody& OtherBody = m_rigidBodies[j];
            COLLISION_STATUS cs = CurBody.GetLinear().GetCollisionStatus(OtherBody.GetLinear());

            switch (cs) {
            case COLLISION_STATUS_TOUCHING:
           //     printf("Collision\n");
                m_rigidBodies[i].CalcCollisionReactions(OtherBody);
                break;

            case COLLISION_STATUS_OVERLAPPING:
           //     printf("Overlapping\n");
                HandleOverlappingBodies(DeltaTime, CurBody, OtherBody);
                break;

            case COLLISION_STATUS_NONE:
                // Nothing here
                break;
            }
        }
    }
}


PointMass* System::AllocPointMass()
{
    if (m_numActivePointMasses == (int)m_pointMasses.size()) {
        printf("Out of point masses\n");
        assert(0);
    }

    PointMass* pm = &m_pointMasses[m_numActivePointMasses];

    m_numActivePointMasses++;

    return pm;
}


RigidBody* System::AllocRigidBody()
{
    if (m_numActiveRigidBodies == (int)m_rigidBodies.size()) {
        printf("Out of rigid bodies\n");
        assert(0);
    }

    RigidBody* rb = &m_rigidBodies[m_numActiveRigidBodies];

    m_numActiveRigidBodies++;

    return rb;
}

 
}
