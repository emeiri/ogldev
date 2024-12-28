/*
    Copyright 2024 Etay Meiri

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

#include "ogldev_color_technique.h"


ColorTechnique::ColorTechnique()
{
}

bool ColorTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/wvp.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/color.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    GET_UNIFORM_AND_CHECK(m_WVPLoc, "gWVP");
    GET_UNIFORM_AND_CHECK(m_colorLoc, "gColor");

    return true;
}


void ColorTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void ColorTechnique::SetColor(const Vector4f& Color)
{
    glUniform4f(m_colorLoc, Color.r, Color.g, Color.b, Color.a);
}
