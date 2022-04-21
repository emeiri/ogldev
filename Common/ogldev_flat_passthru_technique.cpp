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
#include "ogldev_flat_passthru_technique.h"




FlatPassThruTechnique::FlatPassThruTechnique()
{
}

bool FlatPassThruTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/flat_passthru.vs")) {
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

    GLuint BasePosLoc = GetUniformLocation("gQuads[0].BasePos");
    GLuint WidthHeightLoc = GetUniformLocation("gQuads[0].WidthHeight");

    if ((BasePosLoc == INVALID_UNIFORM_LOCATION) || (WidthHeightLoc == INVALID_UNIFORM_LOCATION)) {
        return false;
    }

    glUniform2f(BasePosLoc, 0.0f, 0.0f);
    glUniform2f(WidthHeightLoc, 1.0f, 1.0f);

    glUseProgram(0);

    return true;
}


void FlatPassThruTechnique::SetColor(float r, float g, float b)
{
    glUniform3f(m_colorLoc, r, g, b);
}
