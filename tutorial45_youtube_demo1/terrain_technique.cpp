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

    if (!AddShader(GL_GEOMETRY_SHADER, "terrain.gs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "terrain.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_VPLoc = GetUniformLocation("gVP");
    m_texUnitLoc = GetUniformLocation("gTextureHeight0");    
    m_reversedLightDirLoc = GetUniformLocation("gReversedLightDir");
    m_timeLoc = GetUniformLocation("gTime");

    if (m_VPLoc == INVALID_UNIFORM_LOCATION||
        m_texUnitLoc == INVALID_UNIFORM_LOCATION ||
        m_reversedLightDirLoc == INVALID_UNIFORM_LOCATION ||
        m_timeLoc == INVALID_UNIFORM_LOCATION) {
      //  return false;
    }

    Enable();

    glUniform1i(m_texUnitLoc, COLOR_TEXTURE_UNIT_INDEX_0);

    glUseProgram(0);

    return true;
}


void TerrainTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(m_VPLoc, 1, GL_TRUE, (const GLfloat*)VP.m);
}


void TerrainTechnique::SetTextureHeights(float Tex0Height, float Tex1Height, float Tex2Height, float Tex3Height)
{
    // not used in this demo
}


void TerrainTechnique::SetLightDir(const Vector3f& Dir)
{
    Vector3f ReversedLightDir = Dir * -1.0f;
    ReversedLightDir = ReversedLightDir.Normalize();
    glUniform3f(m_reversedLightDirLoc, ReversedLightDir.x, ReversedLightDir.y, ReversedLightDir.z);
}


void TerrainTechnique::SetTime(float Time)
{
    glUniform1f(m_timeLoc, Time);
}
