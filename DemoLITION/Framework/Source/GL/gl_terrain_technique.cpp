/*

        Copyright 2026 Etay Meiri

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


#include "GL/gl_terrain_technique.h"


bool TerrainTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/terrain.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/terrain.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return InitCommon();
}


bool TerrainTechnique::InitCommon()
{
    GET_UNIFORM(gWVP);
    GET_UNIFORM(gMaxTerrainHeight);
    GET_UNIFORM(gHorizontalScale);
    GET_UNIFORM(gLowHeightPercent);
    GET_UNIFORM(gHighHeightPercent);
    GET_UNIFORM(gSunlightDir);
    GET_UNIFORM(gAmbientFactor);
    GET_UNIFORM(gTexCoordScale);

    return true;
}


void TerrainTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_gWVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void TerrainTechnique::SetMaxTerrainHeight(float MaxHeight)
{
    glUniform1f(m_gMaxTerrainHeightLoc, MaxHeight);
}


void TerrainTechnique::SetHorizontalScale(float HorizontalScale)
{
    glUniform1f(m_gHorizontalScaleLoc, HorizontalScale);
}


void TerrainTechnique::SetHeightPercents(float LowPercent, float HighPercent)
{
    glUniform1f(m_gLowHeightPercentLoc, LowPercent);
    glUniform1f(m_gHighHeightPercentLoc, HighPercent);
}


void TerrainTechnique::SetSunlightDir(const Vector3f& SunlightDir)
{
    // Flipping the Z is a hack - ImGui uses a right handed system
    glUniform3f(m_gSunlightDirLoc, SunlightDir.x, SunlightDir.y, -SunlightDir.z);
}


void TerrainTechnique::SetAmbientLightFactor(float AmbientLightFactor)
{
    glUniform1f(m_gAmbientFactorLoc, AmbientLightFactor);
}


void TerrainTechnique::SetTexTileSizeInWorldUnits(float TexTileSizeInWorldUnits)
{
    glUniform1f(m_gTexCoordScaleLoc, 1.0f / TexTileSizeInWorldUnits);
}

