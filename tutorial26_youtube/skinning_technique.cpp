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

#include "skinning_technique.h"

void DirectionalLight::CalcLocalDirection(const WorldTrans& worldTransform)
{
    LocalDirection = worldTransform.WorldDirToLocalDir(WorldDirection);
}


void PointLight::CalcLocalPosition(const WorldTrans& worldTransform)
{
    LocalPosition = worldTransform.WorldPosToLocalPos(WorldPosition);
}


void SpotLight::CalcLocalDirectionAndPosition(const WorldTrans& worldTransform)
{
    CalcLocalPosition(worldTransform);

    LocalDirection = worldTransform.WorldDirToLocalDir(WorldDirection);
}



SkinningTechnique::SkinningTechnique()
{
}

bool SkinningTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "skinning.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "skinning.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    WVPLoc = GetUniformLocation("gWVP");
    samplerLoc = GetUniformLocation("gSampler");
    samplerSpecularExponentLoc = GetUniformLocation("gSamplerSpecularExponent");
    materialLoc.AmbientColor = GetUniformLocation("gMaterial.AmbientColor");
    materialLoc.DiffuseColor = GetUniformLocation("gMaterial.DiffuseColor");
    materialLoc.SpecularColor = GetUniformLocation("gMaterial.SpecularColor");
    dirLightLoc.Color = GetUniformLocation("gDirectionalLight.Base.Color");
    dirLightLoc.AmbientIntensity = GetUniformLocation("gDirectionalLight.Base.AmbientIntensity");
    dirLightLoc.Direction = GetUniformLocation("gDirectionalLight.Direction");
    dirLightLoc.DiffuseIntensity = GetUniformLocation("gDirectionalLight.Base.DiffuseIntensity");
    CameraLocalPosLoc = GetUniformLocation("gCameraLocalPos");
    NumPointLightsLocation = GetUniformLocation("gNumPointLights");
    NumSpotLightsLocation = GetUniformLocation("gNumSpotLights");

    if (WVPLoc == 0xFFFFFFFF ||
        samplerLoc == 0xFFFFFFFF ||
        samplerSpecularExponentLoc == 0xFFFFFFFF ||
        materialLoc.AmbientColor == 0xFFFFFFFF ||
        materialLoc.DiffuseColor == 0xFFFFFFFF ||
        materialLoc.SpecularColor == 0xFFFFFFFF ||
        CameraLocalPosLoc == 0xFFFFFFFF ||
        dirLightLoc.Color == 0xFFFFFFFF ||
        dirLightLoc.DiffuseIntensity == 0xFFFFFFFF ||
        dirLightLoc.Direction == 0xFFFFFFFF ||
        dirLightLoc.AmbientIntensity == 0xFFFFFFFF ||
        NumPointLightsLocation == INVALID_UNIFORM_LOCATION ||
        NumSpotLightsLocation == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }


    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(PointLightsLocation) ; i++) {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
        PointLightsLocation[i].Color = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
        PointLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].LocalPos", i);
        PointLightsLocation[i].Position = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.DiffuseIntensity", i);
        PointLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Constant", i);
        PointLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Linear", i);
        PointLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Exp", i);
        PointLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

        if (PointLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
            return false;
        }
    }

    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(SpotLightsLocation) ; i++) {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Base.Color", i);
        SpotLightsLocation[i].Color = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
        SpotLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.LocalPos", i);
        SpotLightsLocation[i].Position = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Direction", i);
        SpotLightsLocation[i].Direction = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Cutoff", i);
        SpotLightsLocation[i].Cutoff = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
        SpotLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Constant", i);
        SpotLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Linear", i);
        SpotLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Exp", i);
        SpotLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

        if (SpotLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Direction == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Cutoff == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
            return false;
        }
    }

    displayBoneIndexLocation = GetUniformLocation("gDisplayBoneIndex");
    if (displayBoneIndexLocation == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return true;
}

void SkinningTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void SkinningTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(samplerLoc, TextureUnit);
}

void SkinningTechnique::SetSpecularExponentTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(samplerSpecularExponentLoc, TextureUnit);
}


void SkinningTechnique::SetDirectionalLight(const DirectionalLight& Light)
{
    glUniform3f(dirLightLoc.Color, Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(dirLightLoc.AmbientIntensity, Light.AmbientIntensity);
    Vector3f LocalDirection = Light.GetLocalDirection();
    glUniform3f(dirLightLoc.Direction, LocalDirection.x, LocalDirection.y, LocalDirection.z);
    glUniform1f(dirLightLoc.DiffuseIntensity, Light.DiffuseIntensity);
}


void SkinningTechnique::SetCameraLocalPos(const Vector3f& CameraLocalPos)
{
    glUniform3f(CameraLocalPosLoc, CameraLocalPos.x, CameraLocalPos.y, CameraLocalPos.z);
}


void SkinningTechnique::SetMaterial(const Material& material)
{
    glUniform3f(materialLoc.AmbientColor, material.AmbientColor.r, material.AmbientColor.g, material.AmbientColor.b);
    glUniform3f(materialLoc.DiffuseColor, material.DiffuseColor.r, material.DiffuseColor.g, material.DiffuseColor.b);
    glUniform3f(materialLoc.SpecularColor, material.SpecularColor.r, material.SpecularColor.g, material.SpecularColor.b);
}

void SkinningTechnique::SetPointLights(unsigned int NumLights, const PointLight* pLights)
{
    glUniform1i(NumPointLightsLocation, NumLights);

    for (unsigned int i = 0 ; i < NumLights ; i++) {
        glUniform3f(PointLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(PointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(PointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        const Vector3f& LocalPos = pLights[i].GetLocalPosition();
        //LocalPos.Print();printf("\n");
        glUniform3f(PointLightsLocation[i].Position, LocalPos.x, LocalPos.y, LocalPos.z);
        glUniform1f(PointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(PointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
        glUniform1f(PointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
    }
}

void SkinningTechnique::SetSpotLights(unsigned int NumLights, const SpotLight* pLights)
{
    glUniform1i(NumSpotLightsLocation, NumLights);

    for (unsigned int i = 0 ; i < NumLights ; i++) {
        glUniform3f(SpotLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(SpotLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(SpotLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        const Vector3f& LocalPos = pLights[i].GetLocalPosition();
        glUniform3f(SpotLightsLocation[i].Position, LocalPos.x, LocalPos.y, LocalPos.z);
        Vector3f Direction = pLights[i].GetLocalDirection();
        Direction.Normalize();
        glUniform3f(SpotLightsLocation[i].Direction, Direction.x, Direction.y, Direction.z);
        glUniform1f(SpotLightsLocation[i].Cutoff, cosf(ToRadian(pLights[i].Cutoff)));
        glUniform1f(SpotLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(SpotLightsLocation[i].Atten.Linear,   pLights[i].Attenuation.Linear);
        glUniform1f(SpotLightsLocation[i].Atten.Exp,      pLights[i].Attenuation.Exp);
    }
}


void SkinningTechnique::SetDisplayBoneIndex(uint DisplayBoneIndex)
{
    glUniform1i(displayBoneIndexLocation, DisplayBoneIndex);
}
