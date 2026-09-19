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


class AccelorObject
{
public:

    AccelorObject() = default;

    AccelorObject(float MaxSpeed, float Acceleration, float Deceleration)
    {
        Init(MaxSpeed, Acceleration, Deceleration);
    }

    void Init(float MaxSpeed, float Acceleration, float Deceleration)
    {
        m_maxSpeed = MaxSpeed;
        m_acceleration = Acceleration;
        m_deceleration = Deceleration;
    }


    float Update(float dt, bool IsMovingForward, bool IsMovingBackward)
    {
        if (IsMovingForward) {
            // Accelerate gradually up to max speed
            if (m_currentSpeed < 0.0f) {
                m_currentSpeed = 0;
            }

            m_currentSpeed += m_acceleration * dt;

            if (m_currentSpeed > m_maxSpeed) {
                m_currentSpeed = m_maxSpeed;
            }
        } else if (IsMovingBackward) {
            // Accelerate gradually up to max speed in the opposite direction
            if (m_currentSpeed > 0.0f) {
                m_currentSpeed = 0;
            }

            m_currentSpeed -= m_acceleration * dt;
            if (m_currentSpeed < -m_maxSpeed) {
                m_currentSpeed = -m_maxSpeed;
            }
        } else {
            // Decelerate gradually down to zero
            if (m_currentSpeed > 0.0f) {
                m_currentSpeed -= m_deceleration * dt;
                if (m_currentSpeed < 0.0f) {
                    m_currentSpeed = 0.0f;
                }
            } else if (m_currentSpeed < 0.0f) {
                m_currentSpeed += m_deceleration * dt;
                if (m_currentSpeed > 0.0f) {
                    m_currentSpeed = 0.0f;
                }
            }
        }

        return m_currentSpeed * dt;
    }

    float GetCurrentSpeed() const { return m_currentSpeed; }

    void SetMaxSpeed(float MaxSpeed) { m_maxSpeed = MaxSpeed; }
    void SetAcceleration(float Acceleration) { m_acceleration = Acceleration; }
    void SetDeceleration(float Deceleration) { m_deceleration = Deceleration; }

private:

    float m_currentSpeed = 0.0f;
    float m_maxSpeed = 6.0f;
    float m_acceleration = 3.0f;
    float m_deceleration = 5.0f;
};
