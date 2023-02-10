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
    m_tex0UnitLoc = GetUniformLocation("gTextureHeight0");
    m_tex1UnitLoc = GetUniformLocation("gTextureHeight1");
    m_tex2UnitLoc = GetUniformLocation("gTextureHeight2");
    m_tex3UnitLoc = GetUniformLocation("gTextureHeight3");
    m_tex0HeightLoc = GetUniformLocation("gHeight0");
    m_tex1HeightLoc = GetUniformLocation("gHeight1");
    m_tex2HeightLoc = GetUniformLocation("gHeight2");
    m_tex3HeightLoc = GetUniformLocation("gHeight3");

    if (m_VPLoc == INVALID_UNIFORM_LOCATION||
        m_tex0UnitLoc == INVALID_UNIFORM_LOCATION ||
        m_tex1UnitLoc == INVALID_UNIFORM_LOCATION ||
        m_tex2UnitLoc == INVALID_UNIFORM_LOCATION ||
        m_tex3UnitLoc == INVALID_UNIFORM_LOCATION ||
        m_tex0HeightLoc == INVALID_UNIFORM_LOCATION ||
        m_tex1HeightLoc == INVALID_UNIFORM_LOCATION ||
        m_tex2HeightLoc == INVALID_UNIFORM_LOCATION ||
        m_tex3HeightLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    Enable();

    glUniform1i(m_tex0UnitLoc, COLOR_TEXTURE_UNIT_INDEX_0);
    glUniform1i(m_tex1UnitLoc, COLOR_TEXTURE_UNIT_INDEX_1);
    glUniform1i(m_tex2UnitLoc, COLOR_TEXTURE_UNIT_INDEX_2);
    glUniform1i(m_tex3UnitLoc, COLOR_TEXTURE_UNIT_INDEX_3);

    glUseProgram(0);

    return true;
}


void TerrainTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(m_VPLoc, 1, GL_TRUE, (const GLfloat*)VP.m);
}


void TerrainTechnique::SetTextureHeights(float Tex0Height, float Tex1Height, float Tex2Height, float Tex3Height)
{
    glUniform1f(m_tex0HeightLoc, Tex0Height); 
    glUniform1f(m_tex1HeightLoc, Tex1Height);
    glUniform1f(m_tex2HeightLoc, Tex2Height);
    glUniform1f(m_tex3HeightLoc, Tex3Height);
}



