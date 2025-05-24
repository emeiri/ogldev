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

#include "cloth_technique.h"


ClothTechnique::ClothTechnique()
{
}

bool ClothTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_COMPUTE_SHADER, "cloth.cs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    GET_UNIFORM_AND_CHECK(m_restLengthHorizLoc, "RestLengthHoriz");
    GET_UNIFORM_AND_CHECK(m_restLengthVertLoc, "RestLengthVert");
    GET_UNIFORM_AND_CHECK(m_restLengthDiagLoc, "RestLengthDiag");

    return true;
}

void ClothTechnique::SetRestLengthHoriz(float f)
{
    glUniform1f(m_restLengthHorizLoc, f);
}

void ClothTechnique::SetRestLengthVert(float f)
{
    glUniform1f(m_restLengthVertLoc, f);
}

void ClothTechnique::SetRestLengthDiag(float f)
{
    glUniform1f(m_restLengthDiagLoc, f);
}