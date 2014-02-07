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

#include <limits.h>
#include <string>
#include <glfx.h>
#include <assert.h>

#include "shadow_volume_technique.h"

using namespace std;

ShadowVolumeTechnique::ShadowVolumeTechnique()
{   
}


bool ShadowVolumeTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "shaders/shadow_volume.vs")) {
        return false;
    }

    if (!AddShader(GL_GEOMETRY_SHADER, "shaders/shadow_volume.gs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "shaders/shadow_volume.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }
    
    m_VPLocation = GetUniformLocation("gVP");
    m_WorldMatrixLocation = GetUniformLocation("gWorld");
    m_lightPosLocation = GetUniformLocation("gLightPos");

    if (m_VPLocation == INVALID_UNIFORM_LOCATION ||
        m_WorldMatrixLocation == INVALID_UNIFORM_LOCATION ||
        m_lightPosLocation == INVALID_UNIFORM_LOCATION) {
        return false;
    }
            
    return true;
}


void ShadowVolumeTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(m_VPLocation, 1, GL_TRUE, (const GLfloat*)VP.m);    
}


void ShadowVolumeTechnique::SetWorldMatrix(const Matrix4f& WorldInverse)
{
    glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_TRUE, (const GLfloat*)WorldInverse.m);
}


void ShadowVolumeTechnique::SetLightPos(const Vector3f& Pos)
{
    glUniform3f(m_lightPosLocation, Pos.x, Pos.y, Pos.z);
}