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

#include "ogldev_shadow_mapping_technique_point_light.h"

ShadowMappingPointLightTechnique::ShadowMappingPointLightTechnique()
{

}


bool ShadowMappingPointLightTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/shadow_map_point_light.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/shadow_map_point_light.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_WVPLoc = GetUniformLocation("gWVP");
    m_worldMatrixLoc = GetUniformLocation("gWorld");
    m_lightWorldPosLoc = GetUniformLocation("gLightWorldPos");

    if (m_WVPLoc == INVALID_UNIFORM_LOCATION ||
        m_worldMatrixLoc == INVALID_UNIFORM_LOCATION ||
        m_lightWorldPosLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return true;
}


void ShadowMappingPointLightTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void ShadowMappingPointLightTechnique::SetWorld(const Matrix4f& World)
{
    glUniformMatrix4fv(m_worldMatrixLoc, 1, GL_TRUE, (const GLfloat*)World.m);
}


void ShadowMappingPointLightTechnique::SetLightWorldPos(const Vector3f& Pos)
{
    glUniform3f(m_lightWorldPosLoc, Pos.x, Pos.y, Pos.z);
}
