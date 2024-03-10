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
#include "simple_water_technique.h"


SimpleWaterTechnique::SimpleWaterTechnique()
{
}


bool SimpleWaterTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "simple_water.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "simple_water.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_VPLoc = GetUniformLocation("gVP");
    m_reflectionTexUnitLoc = GetUniformLocation("gReflectionTexture");
    m_refractionTexUnitLoc = GetUniformLocation("gRefractionTexture");
    m_dudvMapTexUnitLoc = GetUniformLocation("gDUDVMapTexture");
    m_normalMapTexUnitLoc = GetUniformLocation("gNormalMap");
    m_depthMapTexUnitLoc = GetUniformLocation("gDepthMap");
    m_heightLoc = GetUniformLocation("gHeight");
    m_dudvOffsetLoc = GetUniformLocation("gDUDVOffset");
    m_cameraPosLoc = GetUniformLocation("gCameraPos");
    m_lightColorLoc = GetUniformLocation("gLightColor");
    m_reversedLightDirLoc = GetUniformLocation("gReversedLightDir");

    if (m_VPLoc == INVALID_UNIFORM_LOCATION ||
        m_heightLoc == INVALID_UNIFORM_LOCATION ||
        m_reflectionTexUnitLoc == INVALID_UNIFORM_LOCATION ||
        m_refractionTexUnitLoc == INVALID_UNIFORM_LOCATION ||
        m_normalMapTexUnitLoc == INVALID_UNIFORM_LOCATION ||
        m_dudvMapTexUnitLoc == INVALID_UNIFORM_LOCATION ||
        m_depthMapTexUnitLoc == INVALID_UNIFORM_LOCATION ||
        m_dudvOffsetLoc == INVALID_UNIFORM_LOCATION ||
        m_cameraPosLoc == INVALID_UNIFORM_LOCATION ||
        m_lightColorLoc == INVALID_UNIFORM_LOCATION ||
        m_reversedLightDirLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return true;
}


void SimpleWaterTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(m_VPLoc, 1, GL_TRUE, (const GLfloat*)VP.m);
}


void SimpleWaterTechnique::SetReflectionTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_reflectionTexUnitLoc, TextureUnit);
}


void SimpleWaterTechnique::SetRefractionTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_refractionTexUnitLoc, TextureUnit);
}


void SimpleWaterTechnique::SetWaterHeight(float Height)
{
    glUniform1f(m_heightLoc, Height);
}


void SimpleWaterTechnique::SetDUDVMapTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_dudvMapTexUnitLoc, TextureUnit);
}


void SimpleWaterTechnique::SetDUDVOffset(float Offset)
{
    glUniform1f(m_dudvOffsetLoc, Offset);
}


void SimpleWaterTechnique::SetCameraPos(const Vector3f& CameraPos)
{
    glUniform3f(m_cameraPosLoc, CameraPos.x, CameraPos.y, CameraPos.z);
}


void SimpleWaterTechnique::SetNormalMapTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_normalMapTexUnitLoc, TextureUnit);
}


void SimpleWaterTechnique::SetDepthMapTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_depthMapTexUnitLoc, TextureUnit);
}


void SimpleWaterTechnique::SetLightColor(const Vector3f& LightColor)
{
    glUniform3f(m_lightColorLoc, LightColor.x, LightColor.y, LightColor.z);
}


void SimpleWaterTechnique::SetLightDir(const Vector3f& LightDir)
{
    Vector3f ReversedLightDir = LightDir * -1.0f;
    ReversedLightDir.Normalize();
    glUniform3f(m_reversedLightDirLoc, ReversedLightDir.x, ReversedLightDir.y, ReversedLightDir.z);
}

