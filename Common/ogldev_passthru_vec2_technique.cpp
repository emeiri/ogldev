/*
    Copyright 2022 Etay Meiri

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

#include "ogldev_util.h"
#include "ogldev_passthru_vec2_technique.h"


PassthruVec2Technique::PassthruVec2Technique()
{
}

bool PassthruVec2Technique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/passthru_vec2.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/flat_passthru.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_colorLoc = GetUniformLocation("gColor");

    if (m_colorLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    Enable();

    // default is white
    SetColor(1.0f, 1.0f, 1.0f);

    return true;
}


void PassthruVec2Technique::SetColor(float r, float g, float b)
{
    glUniform3f(m_colorLoc, r, g, b);
}
