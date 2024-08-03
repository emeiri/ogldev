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
#include "particle.h"

namespace OgldevPhysics
{

class Firework : public Particle
{
public:
    Firework() {}

    ~Firework() {}

    bool Update(float dt);

    void SetType(int Type) { m_type = Type; }
    int GetType() const { return m_type; }    

    void SetAge(float Age) { m_age = Age; }
    float GetAge() const { return m_age; }

private:

    int m_type = 0;
    float m_age = 0.0f;
};

struct FireworkPayload
{
    void Set(int Type, uint Count)
    {
        m_type = Type;
        m_count = Count;
    }

    int m_type = 0;
    uint m_count = 0;
};

struct FireworkConfig
{
    int m_type = 0;
    float m_minAge = 0.0f;
    float m_maxAge = 0.0f;
    Vector3f m_minVelocity = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f m_maxVelocity = Vector3f(0.0f, 0.0f, 0.0f);
    float m_damping = 1.0f;
    std::vector<FireworkPayload> m_payloads;

    void Init(uint NumPayloads)
    {
        m_payloads.resize(NumPayloads);
    }

    void SetParams(int Type, float MinAge, float MaxAge, const Vector3f& MinVelocity, const Vector3f& MaxVelocity, float Damping)
    {
        m_type = Type;
        m_minAge = MinAge;
        m_maxAge = MaxAge;
        m_minVelocity = MinVelocity;
        m_maxVelocity = MaxVelocity;
        m_damping = Damping;
    }

    void Create(Firework& firework, const Firework* pParent = NULL) const;
};

}