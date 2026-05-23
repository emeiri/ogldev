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

#include <vector>
#include "point_mass.h"
#include "rigid_body.h"

namespace Physics {

static constexpr glm::vec3 GRAVITY(0.0f, -9.81f, 0.0f);

class System {

public:

    System() {}

    ~System() {}

    void Init(int NumPointMasses, int NumRigidBodies, UpdateListener pUpdateListener, const glm::vec3& GlobalForce);

    void SetListener(UpdateListener pUpdateListener) { m_pUpdateListener = pUpdateListener; }

    void Update(double DeltaTime);


    PointMass* AllocPointMass();

    RigidBody* AllocRigidBody();

private:

    void UpdateInternal(float DeltaTime);

    void UpdatePointMasses(float DeltaTime);

    void UpdateRigidBodies(float DeltaTime);

    void HandlePointMassCollisions();

    void HandleRigidBodyCollisions(float DeltaTime);

    void ApplyGlobalForces();

    void ResetAllForces();

    std::vector<PointMass> m_pointMasses;
    std::vector<RigidBody> m_rigidBodies;
    UpdateListener m_pUpdateListener = NULL;
    int m_numActivePointMasses = 0;
    int m_numActiveRigidBodies = 0;
    glm::vec3 m_globalForce = glm::vec3(0.0f);
};

}