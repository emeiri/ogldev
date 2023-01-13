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
#include "single_tex_terrain_technique.h"
#include "texture_config.h"


SingleTexTerrainTechnique::SingleTexTerrainTechnique()
{
}

bool SingleTexTerrainTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "single_tex_terrain.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "single_tex_terrain.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_VPLoc = GetUniformLocation("gVP");
    m_texUnitLoc = GetUniformLocation("gTerrainTexture");
    m_minHeightLoc = GetUniformLocation("gMinHeight");
    m_maxHeightLoc = GetUniformLocation("gMaxHeight");

    if (m_VPLoc == INVALID_UNIFORM_LOCATION ||
        m_texUnitLoc == INVALID_UNIFORM_LOCATION ||
        m_minHeightLoc == INVALID_UNIFORM_LOCATION ||
        m_maxHeightLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    Enable();

    glUniform1i(m_texUnitLoc, COLOR_TEXTURE_UNIT_INDEX_0);

    return true;
}


void SingleTexTerrainTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(m_VPLoc, 1, GL_TRUE, (const GLfloat*)VP.m);
}


void SingleTexTerrainTechnique::SetMinMaxHeight(float Min, float Max)
{
    glUniform1f(m_minHeightLoc, Min);
    glUniform1f(m_maxHeightLoc, Max);
}



