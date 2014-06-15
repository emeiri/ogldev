/*

	Copyright 2011 Etay Meiri

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

#include "shadow_map_technique.h"
#include "pipeline.h"


ShadowMapTechnique::ShadowMapTechnique()
{
}

bool ShadowMapTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "shaders/shadow_map.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "shaders/shadow_map.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_WVPLocation = GetUniformLocation("gWVP");
    m_WorldMatrixLocation = GetUniformLocation("gWorld");
    m_lightWorldPosLoc = GetUniformLocation("gLightWorldPos");

    if (m_WVPLocation == INVALID_UNIFORM_LOCATION ||
	m_WorldMatrixLocation == INVALID_UNIFORM_LOCATION ||
        m_lightWorldPosLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return true;
}


void ShadowMapTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void ShadowMapTechnique::SetWorld(const Matrix4f& World)
{
    glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_TRUE, (const GLfloat*)World.m);
}

void ShadowMapTechnique::SetLightWorldPos(const Vector3f& Pos)
{
    glUniform3f(m_lightWorldPosLoc, Pos.x, Pos.y, Pos.z);    
}