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

#include "bindless_tex_technique.h"


BindlessTextureTechnique::BindlessTextureTechnique()
{
}

bool BindlessTextureTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/quad.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bindless_texture.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }


    GET_UNIFORM_AND_CHECK(m_texIndexLoc, "gTextureIndex");

    return true;
}


void BindlessTextureTechnique::SetTextureIndex(int Index)
{
    glUniform1i(m_texIndexLoc, Index);
}


