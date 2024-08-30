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

#include "ogldev_math_3d.h"
#include "particle.h"

namespace OgldevPhysics
{

class ParticleContact
{
public:

    Particle* m_pParticles[2] = { NULL, NULL };

    float m_restitution = 0.0f;

    Vector3f m_contactNormal;

    float m_penetration = 0.0f;

    Vector3f m_particleMovement[2] = { 0.0f };

protected:
    void Resolve(float dt);

    float CalcSeparatingVelocity() const;

private:

    void ResolveVelocity(float dt);

    void ResolveInterpenetration(float dt);
};



}