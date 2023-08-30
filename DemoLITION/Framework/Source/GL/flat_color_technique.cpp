/*

        Copyright 2023 Etay Meiri

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


#include "GL/flat_color_technique.h"


bool FlatColorTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/flat_color.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/flat_color.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return InitCommon();
}


bool FlatColorTechnique::InitCommon()
{
    m_wvpLoc = GetUniformLocation("gWVP");
    m_colorLoc = GetUniformLocation("gColor");

    if (m_wvpLoc == INVALID_UNIFORM_LOCATION ||
        m_colorLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return true;
}


void FlatColorTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_wvpLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void FlatColorTechnique::SetColor(const Vector4f& Color)
{
    glUniform4f(m_colorLoc, Color.x, Color.y, Color.z, Color.w);
}


