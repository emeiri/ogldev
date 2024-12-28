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

#include "particles_technique.h"


ParticlesTechnique::ParticlesTechnique()
{
}

bool ParticlesTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_COMPUTE_SHADER, "particles.cs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    GET_UNIFORM_AND_CHECK(m_blackHole1Loc, "BlackHolePos1");
    GET_UNIFORM_AND_CHECK(m_blackHole2Loc, "BlackHolePos2");

    return true;
}


void ParticlesTechnique::SetBlackHoles(const Vector3f& Pos1, const Vector3f& Pos2)
{
    glUniform3f(m_blackHole1Loc, Pos1.x, Pos1.y, Pos1.z);
    glUniform3f(m_blackHole2Loc, Pos2.x, Pos2.y, Pos2.z);
}
