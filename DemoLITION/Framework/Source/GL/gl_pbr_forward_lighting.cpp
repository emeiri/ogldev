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

//#define FAIL_ON_MISSING_LOC


#include "GL/gl_pbr_forward_lighting.h"

PBRForwardLightingTechnique::PBRForwardLightingTechnique()
{
}


bool PBRForwardLightingTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/forward_lighting.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/pbr_forward_lighting.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    if (!BaseLightingTechnique::Init()) {
        return false;
    }

    GET_UNIFORM_AND_CHECK(m_aoUnitLoc, "gAmbientOcclusion");
    GET_UNIFORM_AND_CHECK(m_emissiveUnitLoc, "gEmissive");
    GET_UNIFORM_AND_CHECK(m_albedoUnitLoc, "gAlbedo");
    GET_UNIFORM_AND_CHECK(m_roughnessUnitLoc, "gRoughness");
    GET_UNIFORM_AND_CHECK(m_envmapUnitLoc, "gEnvMap");
    GET_UNIFORM_AND_CHECK(m_brdfLUTUnitLoc, "gBRDF_LUT");
    GET_UNIFORM_AND_CHECK(m_irradianceUnitLoc, "gIrradiance");
    GET_UNIFORM_AND_CHECK(m_baseColorLoc, "gBaseColor");
    GET_UNIFORM_AND_CHECK(m_emissiveColorLoc, "gEmissiveColor");
    GET_UNIFORM_AND_CHECK(m_metallicRoughnessNormalOcclusionLoc, "gMetallicRoughnessNormalOcclusion");

    return true;
}


void PBRForwardLightingTechnique::SetAmbientOcclusionTextureUnit(int TextureUnit)
{
    glUniform1i(m_aoUnitLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetEmissiveTextureUnit(int TextureUnit)
{
    glUniform1i(m_emissiveUnitLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetAlbedoTextureUnit(int TextureUnit)
{
    glUniform1i(m_albedoUnitLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetRoughnessTextureUnit(int TextureUnit)
{
    glUniform1i(m_roughnessUnitLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetNormalTextureUnit(int TextureUnit)
{
    glUniform1i(m_normalUnitLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetEnvmapTextureUnit(int TextureUnit)
{
    glUniform1i(m_envmapUnitLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetBRDF_LUTTextureUnit(int TextureUnit)
{
    glUniform1i(m_brdfLUTUnitLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetIrradianceTextureUnit(int TextureUnit)
{
    glUniform1i(m_irradianceUnitLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetClearCoatTextureUnit(int TextureUnit)
{
    glUniform1i(m_clearCoatLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetClearCoatRoughnessTextureUnit(int TextureUnit)
{
    glUniform1i(m_clearCoatRoughnessLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetClearCoatNormalTextureUnit(int TextureUnit)
{
    glUniform1i(m_clearCoatNormalLoc, TextureUnit);
}


void PBRForwardLightingTechnique::SetMaterial(const Material& mat)
{
    glUniform4f(m_baseColorLoc, mat.BaseColor.r, mat.BaseColor.g, mat.BaseColor.b, mat.BaseColor.a);
    glUniform4f(m_emissiveColorLoc, mat.EmissiveColor.r, mat.EmissiveColor.g, mat.EmissiveColor.b, mat.EmissiveColor.a);
    glUniform4f(m_metallicRoughnessNormalOcclusionLoc,
                mat.MetallicRoughnessNormalOcclusion.r,
                mat.MetallicRoughnessNormalOcclusion.g,
                mat.MetallicRoughnessNormalOcclusion.b,
                mat.MetallicRoughnessNormalOcclusion.a);
    glUniform4f(m_clearCoatTransmissionThicknessLoc,
                mat.ClearCoatTransmissionThickness.r,
                mat.ClearCoatTransmissionThickness.g,
                mat.ClearCoatTransmissionThickness.b,
                mat.ClearCoatTransmissionThickness.a);
}