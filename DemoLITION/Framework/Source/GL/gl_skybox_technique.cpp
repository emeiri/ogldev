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


#include "GL/gl_skybox_technique.h"


bool SkyboxTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/skybox.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/skybox.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return InitCommon();
}


bool SkyboxTechnique::InitCommon()
{
    GET_UNIFORM_AND_CHECK(m_VPLoc, "gVP");
    GET_UNIFORM_AND_CHECK(m_cubemapSamplerLoc, "gCubemapTexture");

    return true;
}


void SkyboxTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(m_VPLoc, 1, GL_TRUE, (const GLfloat*)VP.m);
}


void SkyboxTechnique::SetTextureUnitIndex(int TextureUnitIndex)
{
    glUniform1i(m_cubemapSamplerLoc, TextureUnitIndex);
}




