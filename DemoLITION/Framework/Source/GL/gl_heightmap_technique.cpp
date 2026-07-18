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


#include "GL/gl_heightmap_technique.h"


bool HeightmapTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/heightmap.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/heightmap.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return InitCommon();
}


bool HeightmapTechnique::InitCommon()
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


void HeightmapTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_gWVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void HeightmapTechnique::SetMaxTerrainHeight(float MaxHeight)
{
    glUniform1f(m_gMaxTerrainHeightLoc, MaxHeight);
}


void HeightmapTechnique::SetHorizontalScale(float HorizontalScale)
{
    glUniform1f(m_gHorizontalScaleLoc, HorizontalScale);
}


void HeightmapTechnique::SetHeightPercents(float LowPercent, float HighPercent)
{
    glUniform1f(m_gLowHeightPercentLoc, LowPercent);
    glUniform1f(m_gHighHeightPercentLoc, HighPercent);
}


void HeightmapTechnique::SetSunlightDir(const Vector3f& SunlightDir)
{
    // Flipping the Z is a hack - ImGui uses a right handed system
    glUniform3f(m_gSunlightDirLoc, SunlightDir.x, SunlightDir.y, -SunlightDir.z);
}


void HeightmapTechnique::SetAmbientLightFactor(float AmbientLightFactor)
{
    glUniform1f(m_gAmbientFactorLoc, AmbientLightFactor);
}


void HeightmapTechnique::SetTexTileSizeInWorldUnits(float TexTileSizeInWorldUnits)
{
    glUniform1f(m_gTexCoordScaleLoc, 1.0f / TexTileSizeInWorldUnits);
}

