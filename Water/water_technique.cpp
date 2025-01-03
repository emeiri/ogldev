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

#include "water_technique.h"


WaterTechnique::WaterTechnique()
{
}

bool WaterTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/square.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "water.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    GET_UNIFORM_AND_CHECK(m_WVPLoc, "gWVP");
    GET_UNIFORM_AND_CHECK(m_timeLoc, "iTime");

    return true;
}


void WaterTechnique::Render(const Matrix4f& WVP, float Time)
{
    Enable();

    glUniformMatrix4fv(m_WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
    glUniform1f(m_timeLoc, Time);

    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
}


