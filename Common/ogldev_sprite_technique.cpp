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

    m_samplerLoc = GetUniformLocation("gSampler");

    if (m_samplerLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    char name[200] = { 0 };

    for (int i = 0 ; i < SPRITE_TECH_MAX_QUADS ; i++) {
        SNPRINTF(name, sizeof(name), "gQuads[%d].BasePos", i);
        m_quadsLoc[i].BasePos = GetUniformLocation(name);
        SNPRINTF(name, sizeof(name), "gQuads[%d].WidthHeight", i);
        m_quadsLoc[i].WidthHeight = GetUniformLocation(name);
        SNPRINTF(name, sizeof(name), "gQuads[%d].TexCoords", i);
        m_quadsLoc[i].TexCoords = GetUniformLocation(name);
        SNPRINTF(name, sizeof(name), "gQuads[%d].TexWidthHeight", i);
        m_quadsLoc[i].TexWidthHeight = GetUniformLocation(name);

        if ((m_quadsLoc[i].BasePos == INVALID_UNIFORM_LOCATION) ||
            (m_quadsLoc[i].WidthHeight == INVALID_UNIFORM_LOCATION) ||
            (m_quadsLoc[i].TexCoords == INVALID_UNIFORM_LOCATION) ||
            (m_quadsLoc[i].TexWidthHeight == INVALID_UNIFORM_LOCATION)) {
            return false;
        }
    }

    return true;
}


void SpriteTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_samplerLoc, TextureUnit);
}


void SpriteTechnique::SetQuad(int Index,
                              float NDCX, float NDCY, float Width, float Height,
                              float u, float v, float TexWidth, float TexHeight)
{
    assert(Index < SPRITE_TECH_MAX_QUADS);

    glUniform2f(m_quadsLoc[Index].BasePos, NDCX, NDCY);
    glUniform2f(m_quadsLoc[Index].WidthHeight, Width, Height);

    glUniform2f(m_quadsLoc[Index].TexCoords, u, v);
    glUniform2f(m_quadsLoc[Index].TexWidthHeight, TexWidth, TexHeight);
}
