/*

        Copyright 2024 Etay Meiri

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

#include <assert.h>

#include "ogldev_math_3d.h"

namespace OgldevPhysics
{

class Particle {

public:

    const Vector3f& GetPosition() const { return m_position; }
    void SetPosition(const Vector3f& Position) { m_position = Position; }    
    void SetPosition(float x, float y, float z) { m_position.x = x; m_position.y = y; m_position.z = z; }

    float GetMass() const;
    void SetMass(float Mass);

    float GetReciprocalMass() const { return m_reciprocalMass; }
    void SetReciprocalMass(float ReciprocalMass) { m_reciprocalMass = ReciprocalMass; }

    const Vector3f& GetVelocity() const { return m_velocity; }
    void SetVelocity(const Vector3f& Velocity) { m_velocity = Velocity; }

    const Vector3f& GetAcceleration() const { return m_acceleration; }
    void SetAcceleration(const Vector3f& Acceleration) { m_acceleration = Acceleration; }

    void SetDamping(float Damping) { m_damping = Damping; }

    void Integrate(float dt);

    void AddForce(const Vector3f& Force) { m_forceAccum += Force; }

    bool HasFiniteMass() const { return (m_reciprocalMass >= 0.0f); }

    void ClearAccum();

protected:    

    Vector3f m_position = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f m_velocity = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f m_acceleration = Vector3f(0.0f, 0.0f, 0.0f);

    Vector3f m_forceAccum = Vector3f(0.0f, 0.0f, 0.0f);

    float m_damping = 0.999f;
    float m_reciprocalMass = 0.0f;
};


}