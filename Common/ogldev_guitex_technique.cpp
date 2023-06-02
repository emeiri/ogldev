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

#include "ogldev_guitex_technique.h"


GUITexTechnique::GUITexTechnique()
{
}

bool GUITexTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/guitex.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/guitex.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_samplerLoc = GetUniformLocation("gSampler");

    if (m_samplerLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return true;
}


void GUITexTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_samplerLoc, TextureUnit);
}


