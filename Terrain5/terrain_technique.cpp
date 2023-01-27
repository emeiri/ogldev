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
#include "terrain_technique.h"
#include "texture_config.h"


TerrainTechnique::TerrainTechnique()
{
}

bool TerrainTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "terrain.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "terrain.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_VPLoc = GetUniformLocation("gVP");
    m_minHeightLoc = GetUniformLocation("gMinHeight");
    m_maxHeightLoc = GetUniformLocation("gMaxHeight");
    m_texHeight0UnitLoc = GetUniformLocation("gTextureHeight0");
    m_texHeight1UnitLoc = GetUniformLocation("gTextureHeight1");
    m_texHeight2UnitLoc = GetUniformLocation("gTextureHeight2");
    m_texHeight3UnitLoc = GetUniformLocation("gTextureHeight3");

    if (m_VPLoc == INVALID_UNIFORM_LOCATION||
        m_minHeightLoc == INVALID_UNIFORM_LOCATION ||
        m_maxHeightLoc == INVALID_UNIFORM_LOCATION ||
        m_texHeight0UnitLoc == INVALID_UNIFORM_LOCATION ||
        m_texHeight1UnitLoc == INVALID_UNIFORM_LOCATION ||
        m_texHeight2UnitLoc == INVALID_UNIFORM_LOCATION ||
        m_texHeight3UnitLoc == INVALID_UNIFORM_LOCATION) {
     //   return false;
    }

    Enable();

    glUniform1i(m_texHeight0UnitLoc, COLOR_TEXTURE_UNIT_INDEX_0);
    glUniform1i(m_texHeight1UnitLoc, COLOR_TEXTURE_UNIT_INDEX_1);
    glUniform1i(m_texHeight2UnitLoc, COLOR_TEXTURE_UNIT_INDEX_2);
    glUniform1i(m_texHeight3UnitLoc, COLOR_TEXTURE_UNIT_INDEX_3);

    return true;
}


void TerrainTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(m_VPLoc, 1, GL_TRUE, (const GLfloat*)VP.m);
}


void TerrainTechnique::SetMinMaxHeight(float Min, float Max)
{
    glUniform1f(m_minHeightLoc, Min);
    glUniform1f(m_maxHeightLoc, Max);
}
