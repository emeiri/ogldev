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

#include "GL/gl_tone_map_technique.h"


bool ToneMapTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/full_screen_quad.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/tone_map.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    glGenVertexArrays(1, &m_dummyVAO);

    GET_UNIFORM_AND_CHECK(m_avgLumLoc, "gAvgLum");
    GET_UNIFORM_AND_CHECK(m_hdrSamplerLoc, "gHDRSampler");
    GET_UNIFORM_AND_CHECK(m_exposure, "gExposure");
    GET_UNIFORM_AND_CHECK(m_methodTypeLoc, "gMethodType");
    GET_UNIFORM_AND_CHECK(m_enableGammaLoc, "gEnableGammaCorrection");

    return true;
}


void ToneMapTechnique::Render()
{    
    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void ToneMapTechnique::SetAverageLuminance(float AvgLum)
{
    glUniform1f(m_avgLumLoc, AvgLum);
}


void ToneMapTechnique::SetHDRSampler(unsigned int TextureUnit)
{
    glUniform1i(m_hdrSamplerLoc, TextureUnit);
}


void ToneMapTechnique::SetExposure(float Exposure)
{
    glUniform1f(m_exposure, Exposure);
}


void ToneMapTechnique::SetToneMapMethod(TONE_MAP_METHOD Method)
{
    glUniform1i(m_methodTypeLoc, Method);
}


void ToneMapTechnique::ControlGammaCorrection(bool Enable)
{
    glUniform1i(m_enableGammaLoc, Enable);
}

