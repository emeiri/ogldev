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


void DirectionalLight::CalcLocalDirection(const Matrix4f& World)
{
    Matrix3f World3f(World);  // Initialize using the top left corner

    // Inverse local-to-world transformation using transpose
    // (assuming uniform scaling)
    Matrix3f WorldToLocal = World3f.Transpose();

    LocalDirection = WorldToLocal * WorldDirection;

    LocalDirection = LocalDirection.Normalize();
}


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
    materialLoc.AmbientColor = GetUniformLocation("gMaterial.AmbientColor");
    materialLoc.DiffuseColor = GetUniformLocation("gMaterial.DiffuseColor");
    dirLightLoc.Color = GetUniformLocation("gDirectionalLight.Color");
    dirLightLoc.AmbientIntensity = GetUniformLocation("gDirectionalLight.AmbientIntensity");
    dirLightLoc.Direction = GetUniformLocation("gDirectionalLight.Direction");
    dirLightLoc.DiffuseIntensity = GetUniformLocation("gDirectionalLight.DiffuseIntensity");

    if (WVPLoc == 0xFFFFFFFF ||
        samplerLoc == 0xFFFFFFFF ||
        materialLoc.AmbientColor == 0xFFFFFFFF ||
        materialLoc.DiffuseColor == 0xFFFFFFFF ||
        dirLightLoc.Color == 0xFFFFFFFF ||
        dirLightLoc.DiffuseIntensity == 0xFFFFFFFF ||
        dirLightLoc.Direction == 0xFFFFFFFF ||
        dirLightLoc.AmbientIntensity == 0xFFFFFFFF)
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


void LightingTechnique::SetDirectionalLight(const DirectionalLight& Light)
{
    glUniform3f(dirLightLoc.Color, Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(dirLightLoc.AmbientIntensity, Light.AmbientIntensity);
    Vector3f LocalDirection = Light.GetLocalDirection();
    glUniform3f(dirLightLoc.Direction, LocalDirection.x, LocalDirection.y, LocalDirection.z);
    glUniform1f(dirLightLoc.DiffuseIntensity, Light.DiffuseIntensity);
}


void LightingTechnique::SetMaterial(const Material& material)
{
    glUniform3f(materialLoc.AmbientColor, material.AmbientColor.r, material.AmbientColor.g, material.AmbientColor.b);
    glUniform3f(materialLoc.DiffuseColor, material.DiffuseColor.r, material.DiffuseColor.g, material.DiffuseColor.b);
}
