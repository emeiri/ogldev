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
#include "force_generator.h"

namespace OgldevPhysics
{

class BungeeSpringForceGenerator : public ForceGenerator
{
public:

    BungeeSpringForceGenerator() {}

    BungeeSpringForceGenerator(Particle* pOtherEnd, float SpringConstant, float RestLength);

    void Init(Particle* pOtherEnd, float SpringConstant, float RestLength);

    virtual void UpdateForce(Particle* pParticle, float dt);

private:

    Particle* m_pOtherEnd = NULL;

    float m_springConstant = 0.0f;

    float m_restLength = 0.0f;
};

}
