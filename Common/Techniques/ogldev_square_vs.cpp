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

#include "Techniques/ogldev_square_vs.h"


SquareVSTechnique::SquareVSTechnique()
{
}

bool SquareVSTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/square.vs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    GET_UNIFORM_AND_CHECK(m_WVPLoc, "gWVP");

    return true;
}


void SquareVSTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


