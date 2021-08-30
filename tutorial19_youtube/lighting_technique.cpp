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

#include "lighting_technique.h"


LightingTechnique::LightingTechnique()
{
}

bool LightingTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "lighting.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "lighting.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    WVPLoc = GetUniformLocation("gWVP");
    samplerLoc = GetUniformLocation("gSampler");
    lightColorLoc = GetUniformLocation("gLight.Color");
    lightAmbientIntensityLoc = GetUniformLocation("gLight.AmbientIntensity");
    materialAmbientColorLoc = GetUniformLocation("gMaterial.AmbientColor");

    if (lightAmbientIntensityLoc == 0xFFFFFFFF ||
        WVPLoc == 0xFFFFFFFF ||
        samplerLoc == 0xFFFFFFFF ||
        lightColorLoc == 0xFFFFFFFF ||
        materialAmbientColorLoc == 0xFFF)
    {
        return false;
    }

    return true;
}

void LightingTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void LightingTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(samplerLoc, TextureUnit);
}


void LightingTechnique::SetLight(const BaseLight& Light)
{
    glUniform3f(lightColorLoc, Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(lightAmbientIntensityLoc, Light.AmbientIntensity);
}


void LightingTechnique::SetMaterial(const Material& material)
{
    glUniform3f(materialAmbientColorLoc, material.AmbientColor.r, material.AmbientColor.g, material.AmbientColor.b);
}
