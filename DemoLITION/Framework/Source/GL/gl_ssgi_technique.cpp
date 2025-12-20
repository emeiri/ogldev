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

#include "GL/gl_ssgi_technique.h"


bool SSGITechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/full_screen_tri.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/ssgi.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    GET_UNIFORM_AND_CHECK(m_albedoSamplerLoc, "gAlbedoTex");
    GET_UNIFORM_AND_CHECK(m_normalSamplerLoc, "gNormalTex");
    GET_UNIFORM_AND_CHECK(m_depthSamplerLoc, "gDepthTex");
    GET_UNIFORM_AND_CHECK(m_invProjLoc, "gInvProj");

    glGenVertexArrays(1, &m_dummyVAO);

    return true;
}


void SSGITechnique::Render()
{    
    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}


void SSGITechnique::SetAlbedoTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_albedoSamplerLoc, TextureUnit);
}


void SSGITechnique::SetNormalTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_normalSamplerLoc, TextureUnit);
}


void SSGITechnique::SetDepthTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_depthSamplerLoc, TextureUnit);
}


void SSGITechnique::SetInverseProj(const Matrix4f& InvProj)
{
    glUniformMatrix4fv(m_invProjLoc, 1, GL_TRUE, (const GLfloat*)InvProj.m);
}
