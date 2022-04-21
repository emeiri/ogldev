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
#include "ogldev_sprite_technique.h"


SpriteTechnique::SpriteTechnique()
{
}

bool SpriteTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/sprite.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/sprite.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_colorLoc = GetUniformLocation("gColor");

    if (m_colorLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    char name[200] = { 0 };

    for (int i = 0 ; i < MAX_QUADS ; i++) {
        SNPRINTF(name, sizeof(name), "gQuads[%d].BasePos", i);
        m_quadsLoc[i].BasePosLoc = GetUniformLocation(name);
        SNPRINTF(name, sizeof(name), "gQuads[%d].WidthHeight", i);
        m_quadsLoc[i].WidthHeightLoc = GetUniformLocation(name);

        if ((m_quadsLoc[i].BasePosLoc == INVALID_UNIFORM_LOCATION) ||
            (m_quadsLoc[i].WidthHeightLoc == INVALID_UNIFORM_LOCATION)) {
            return false;
        }
    }

    Enable();

    // default is white
    SetColor(1.0f, 1.0f, 1.0f);

    glUseProgram(0);

    return true;
}


void SpriteTechnique::SetColor(float r, float g, float b)
{
    glUniform3f(m_colorLoc, r, g, b);
}


void SpriteTechnique::SetQuad(int Index, float x, float y, float Width, float Height)
{
    assert(Index < MAX_QUADS);

    glUniform2f(m_quadsLoc[Index].BasePosLoc, x, y);
    glUniform2f(m_quadsLoc[Index].WidthHeightLoc, Width, Height);
}
