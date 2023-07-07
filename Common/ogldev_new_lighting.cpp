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

#include "ogldev_new_lighting.h"

#define FAIL_ON_MISSING_LOC

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



LightingTechnique::LightingTechnique()
{
}

bool LightingTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/lighting_new.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/lighting_new.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return InitCommon();
}

bool LightingTechnique::InitCommon()
{
    WVPLoc = GetUniformLocation("gWVP");
    WorldMatrixLoc = GetUniformLocation("gWorld");
    LightWVPLoc = GetUniformLocation("gLightWVP"); // required only for shadow mapping
    samplerLoc = GetUniformLocation("gSampler");
    shadowMapLoc = GetUniformLocation("gShadowMap");
    shadowCubeMapLoc = GetUniformLocation("gShadowCubeMap");
    shadowMapWidthLoc = GetUniformLocation("gShadowMapWidth");
    shadowMapHeightLoc = GetUniformLocation("gShadowMapHeight");
    shadowMapFilterSizeLoc = GetUniformLocation("gShadowMapFilterSize");
    ShadowMapOffsetTextureLoc = GetUniformLocation("gShadowMapOffsetTexture");
    ShadowMapOffsetTextureSizeLoc = GetUniformLocation("gShadowMapOffsetTextureSize");
    ShadowMapOffsetFilterSizeLoc = GetUniformLocation("gShadowMapOffsetFilterSize");
    ShadowMapRandomRadiusLoc = GetUniformLocation("gShadowMapRandomRadius");
    samplerSpecularExponentLoc = GetUniformLocation("gSamplerSpecularExponent");
    materialLoc.AmbientColor = GetUniformLocation("gMaterial.AmbientColor");
    materialLoc.DiffuseColor = GetUniformLocation("gMaterial.DiffuseColor");
    materialLoc.SpecularColor = GetUniformLocation("gMaterial.SpecularColor");
    dirLightLoc.Color = GetUniformLocation("gDirectionalLight.Base.Color");
    dirLightLoc.AmbientIntensity = GetUniformLocation("gDirectionalLight.Base.AmbientIntensity");
    dirLightLoc.Direction = GetUniformLocation("gDirectionalLight.Direction");
    dirLightLoc.DiffuseIntensity = GetUniformLocation("gDirectionalLight.Base.DiffuseIntensity");
    CameraLocalPosLoc = GetUniformLocation("gCameraLocalPos");
    CameraWorldPosLoc = GetUniformLocation("gCameraWorldPos");
    NumPointLightsLoc = GetUniformLocation("gNumPointLights");
    NumSpotLightsLoc = GetUniformLocation("gNumSpotLights");
    ColorModLocation = GetUniformLocation("gColorMod");
    ColorAddLocation = GetUniformLocation("gColorAdd");
    EnableRimLightLoc = GetUniformLocation("gRimLightEnabled");
    EnableCellShadingLoc = GetUniformLocation("gCellShadingEnabled");
    EnableSpecularExponent = GetUniformLocation("gEnableSpecularExponent");
    FogStartLoc = GetUniformLocation("gFogStart");
    FogEndLoc = GetUniformLocation("gFogEnd");
    FogColorLoc = GetUniformLocation("gFogColor");
    ExpFogDensityLoc = GetUniformLocation("gExpFogDensity");
    ExpSquaredFogEnabledLoc = GetUniformLocation("gExpSquaredFogEnabled");
    LayeredFogTopLoc = GetUniformLocation("gLayeredFogTop");
    FogTimeLoc = GetUniformLocation("gFogTime");
    IsPBRLoc = GetUniformLocation("gIsPBR");
    PBRMaterialLoc.Roughness = GetUniformLocation("gPBRmaterial.Roughness");
    PBRMaterialLoc.IsMetal = GetUniformLocation("gPBRmaterial.IsMetal");
    PBRMaterialLoc.Color = GetUniformLocation("gPBRmaterial.Color");
    ClipPlaneLoc = GetUniformLocation("gClipPlane");

    if (WVPLoc == INVALID_UNIFORM_LOCATION ||
        WorldMatrixLoc == INVALID_UNIFORM_LOCATION ||
        LightWVPLoc == INVALID_UNIFORM_LOCATION ||  // required only for shadow mapping
        samplerLoc == INVALID_UNIFORM_LOCATION ||
        shadowMapLoc == INVALID_UNIFORM_LOCATION ||
        shadowCubeMapLoc == INVALID_UNIFORM_LOCATION ||
        shadowMapWidthLoc == INVALID_UNIFORM_LOCATION ||
        shadowMapHeightLoc == INVALID_UNIFORM_LOCATION ||
        shadowMapFilterSizeLoc == INVALID_UNIFORM_LOCATION ||
        ShadowMapOffsetTextureLoc == INVALID_UNIFORM_LOCATION ||
        ShadowMapOffsetTextureSizeLoc == INVALID_UNIFORM_LOCATION ||
        ShadowMapOffsetFilterSizeLoc == INVALID_UNIFORM_LOCATION ||
        ShadowMapRandomRadiusLoc == INVALID_UNIFORM_LOCATION ||
        samplerSpecularExponentLoc == INVALID_UNIFORM_LOCATION ||
        materialLoc.AmbientColor == INVALID_UNIFORM_LOCATION ||
        materialLoc.DiffuseColor == INVALID_UNIFORM_LOCATION ||
        materialLoc.SpecularColor == INVALID_UNIFORM_LOCATION ||
        CameraLocalPosLoc == INVALID_UNIFORM_LOCATION ||
        CameraWorldPosLoc == INVALID_UNIFORM_LOCATION ||
        dirLightLoc.Color == INVALID_UNIFORM_LOCATION ||
        dirLightLoc.DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
        dirLightLoc.Direction == INVALID_UNIFORM_LOCATION ||
        dirLightLoc.AmbientIntensity == INVALID_UNIFORM_LOCATION ||
        NumPointLightsLoc == INVALID_UNIFORM_LOCATION ||
        NumSpotLightsLoc == INVALID_UNIFORM_LOCATION ||
        EnableRimLightLoc == INVALID_UNIFORM_LOCATION ||
        EnableCellShadingLoc == INVALID_UNIFORM_LOCATION ||
        EnableSpecularExponent == INVALID_UNIFORM_LOCATION ||
        FogStartLoc == INVALID_UNIFORM_LOCATION ||
        FogEndLoc == INVALID_UNIFORM_LOCATION ||
        FogColorLoc == INVALID_UNIFORM_LOCATION ||
        ExpFogDensityLoc == INVALID_UNIFORM_LOCATION ||
        ExpSquaredFogEnabledLoc == INVALID_UNIFORM_LOCATION ||
        LayeredFogTopLoc == INVALID_UNIFORM_LOCATION ||
        FogTimeLoc == INVALID_UNIFORM_LOCATION ||
        ColorModLocation == INVALID_UNIFORM_LOCATION ||
        ColorAddLocation == INVALID_UNIFORM_LOCATION ||
        IsPBRLoc == INVALID_UNIFORM_LOCATION ||
        PBRMaterialLoc.Roughness == INVALID_UNIFORM_LOCATION ||
        PBRMaterialLoc.IsMetal == INVALID_UNIFORM_LOCATION ||
        PBRMaterialLoc.Color == INVALID_UNIFORM_LOCATION ||
        ClipPlaneLoc == INVALID_UNIFORM_LOCATION) {

#ifdef FAIL_ON_MISSING_LOC
        return false;
#endif
    }


    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(PointLightsLocation) ; i++) {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
        PointLightsLocation[i].Color = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
        PointLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].LocalPos", i);
        PointLightsLocation[i].LocalPos = GetUniformLocation(Name);

        SNPRINTF(Name, sizeof(Name), "gPointLights[%d].WorldPos", i);
        PointLightsLocation[i].WorldPos = GetUniformLocation(Name);

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
            PointLightsLocation[i].LocalPos == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].WorldPos == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
#ifdef FAIL_ON_MISSING_LOC
            return false;
#endif
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
#ifdef FAIL_ON_MISSING_LOC
            return false;
#endif
        }
    }

    return true;
}


void LightingTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void LightingTechnique::SetWorldMatrix(const Matrix4f& World)
{
    glUniformMatrix4fv(WorldMatrixLoc, 1, GL_TRUE, (const GLfloat*)World.m);
}


void LightingTechnique::SetLightWVP(const Matrix4f& LightWVP)
{
    glUniformMatrix4fv(LightWVPLoc, 1, GL_TRUE, (const GLfloat*)LightWVP.m);
}


void LightingTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(samplerLoc, TextureUnit);
}


void LightingTechnique::SetShadowMapSize(unsigned int Width, unsigned int Height)
{
    glUniform1i(shadowMapWidthLoc, Width);
    glUniform1i(shadowMapHeightLoc, Height);
}


void LightingTechnique::SetShadowMapFilterSize(unsigned int Size)
{
    glUniform1i(shadowMapFilterSizeLoc, Size);
}


void LightingTechnique::SetShadowMapTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(shadowMapLoc, TextureUnit);
}


void LightingTechnique::SetShadowCubeMapTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(shadowCubeMapLoc, TextureUnit);
}


void LightingTechnique::SetShadowMapOffsetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(ShadowMapOffsetTextureLoc, TextureUnit);
}


void LightingTechnique::SetShadowMapOffsetTextureParams(float TextureSize, float FilterSize, float Radius)
{
    glUniform1f(ShadowMapOffsetTextureSizeLoc, TextureSize);
    glUniform1f(ShadowMapOffsetFilterSizeLoc, FilterSize);
    glUniform1f(ShadowMapRandomRadiusLoc, Radius);
}


void LightingTechnique::SetSpecularExponentTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(samplerSpecularExponentLoc, TextureUnit);
}


void LightingTechnique::SetDirectionalLight(const DirectionalLight& DirLight, bool WithDir)
{
    glUniform3f(dirLightLoc.Color, DirLight.Color.x, DirLight.Color.y, DirLight.Color.z);
    glUniform1f(dirLightLoc.AmbientIntensity, DirLight.AmbientIntensity);
    glUniform1f(dirLightLoc.DiffuseIntensity, DirLight.DiffuseIntensity);

    if (WithDir) {
        UpdateDirLightDirection(DirLight);
    }
}


void LightingTechnique::UpdateDirLightDirection(const DirectionalLight& DirLight)
{
    Vector3f LocalDirection = DirLight.GetLocalDirection();

    LocalDirection.Normalize();

    glUniform3f(dirLightLoc.Direction, LocalDirection.x, LocalDirection.y, LocalDirection.z);

    Vector3f WorldDirection = DirLight.WorldDirection;
    WorldDirection.Normalize();
}


void LightingTechnique::SetCameraLocalPos(const Vector3f& CameraLocalPos)
{
    glUniform3f(CameraLocalPosLoc, CameraLocalPos.x, CameraLocalPos.y, CameraLocalPos.z);
}


void LightingTechnique::SetCameraWorldPos(const Vector3f& CameraWorldPos)
{
    glUniform3f(CameraWorldPosLoc, CameraWorldPos.x, CameraWorldPos.y, CameraWorldPos.z);
}


void LightingTechnique::SetMaterial(const Material& material)
{
    glUniform3f(materialLoc.AmbientColor, material.AmbientColor.r, material.AmbientColor.g, material.AmbientColor.b);
    glUniform3f(materialLoc.DiffuseColor, material.DiffuseColor.r, material.DiffuseColor.g, material.DiffuseColor.b);
    glUniform3f(materialLoc.SpecularColor, material.SpecularColor.r, material.SpecularColor.g, material.SpecularColor.b);
}


void LightingTechnique::SetPointLights(unsigned int NumLights, const PointLight* pLights, bool WithPos)
{
    glUniform1i(NumPointLightsLoc, NumLights);

    for (unsigned int i = 0 ; i < NumLights ; i++) {
        glUniform3f(PointLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(PointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(PointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform1f(PointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(PointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
        glUniform1f(PointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
    }

    if (WithPos) {
        UpdatePointLightsPos(NumLights, pLights);
    }
}


void LightingTechnique::UpdatePointLightsPos(unsigned int NumLights, const PointLight* pLights)
{
    for (unsigned int i = 0 ; i < NumLights ; i++) {
        const Vector3f& LocalPos = pLights[i].GetLocalPosition();
        glUniform3f(PointLightsLocation[i].LocalPos, LocalPos.x, LocalPos.y, LocalPos.z);
        const Vector3f& WorldPos = pLights[i].WorldPosition;
        glUniform3f(PointLightsLocation[i].WorldPos, WorldPos.x, WorldPos.y, WorldPos.z);
    }
}

void LightingTechnique::SetSpotLights(unsigned int NumLights, const SpotLight* pLights, bool WithPosAndDir)
{
    glUniform1i(NumSpotLightsLoc, NumLights);

    for (unsigned int i = 0 ; i < NumLights ; i++) {
        glUniform3f(SpotLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(SpotLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(SpotLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform1f(SpotLightsLocation[i].Cutoff, cosf(ToRadian(pLights[i].Cutoff)));
        glUniform1f(SpotLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(SpotLightsLocation[i].Atten.Linear,   pLights[i].Attenuation.Linear);
        glUniform1f(SpotLightsLocation[i].Atten.Exp,      pLights[i].Attenuation.Exp);
    }

    if (WithPosAndDir) {
        UpdateSpotLightsPosAndDir(NumLights, pLights);
    }
}


void LightingTechnique::UpdateSpotLightsPosAndDir(unsigned int NumLights, const SpotLight* pLights)
{
    for (unsigned int i = 0 ; i < NumLights ; i++) {
        const Vector3f& LocalPos = pLights[i].GetLocalPosition();
        glUniform3f(SpotLightsLocation[i].Position, LocalPos.x, LocalPos.y, LocalPos.z);
        Vector3f Direction = pLights[i].GetLocalDirection();
        Direction.Normalize();
        glUniform3f(SpotLightsLocation[i].Direction, Direction.x, Direction.y, Direction.z);
    }
}


void LightingTechnique::SetColorMod(const Vector4f& Color)
{
    glUniform4f(ColorModLocation, Color.x, Color.y, Color.z, Color.w);
}


void LightingTechnique::SetColorAdd(const Vector4f& Color)
{
    glUniform4f(ColorAddLocation, Color.x, Color.y, Color.z, Color.w);
}


void LightingTechnique::ControlRimLight(bool IsEnabled)
{
    if (IsEnabled) {
        glUniform1i(EnableRimLightLoc, 1);
    } else {
        glUniform1i(EnableRimLightLoc, 0);
    }
}


void LightingTechnique::ControlCellShading(bool IsEnabled)
{
    if (IsEnabled) {
        glUniform1i(EnableCellShadingLoc, 1);
    } else {
        glUniform1i(EnableCellShadingLoc, 0);
    }
}


void LightingTechnique::ControlSpecularExponent(bool IsEnabled)
{
    if (IsEnabled) {
        glUniform1i(EnableSpecularExponent, 1);
    } else {
        glUniform1i(EnableSpecularExponent, 0);
    }
}


void LightingTechnique::SetLinearFog(float FogStart, float FogEnd)
{
    if (FogStart < 0.0f) {
        printf("Fog start must be positive: %f\n", FogStart);
        exit(1);
    }

    if (FogEnd < 0.0f) {
        printf("Fog end must be positive: %f\n", FogEnd);
        exit(1);
    }

    if (FogStart >= FogEnd) {
        printf("FogStart %f must be smaller than FogEnd %f\n", FogStart, FogEnd);
        exit(1);
    }

    glUniform1f(LayeredFogTopLoc, -1.0f);
    glUniform1f(FogTimeLoc, -1.0f);

    glUniform1f(FogStartLoc, FogStart);
    glUniform1f(FogEndLoc, FogEnd);
}


void LightingTechnique::SetExpFog(float FogEnd, float FogDensity)
{
    SetExpFogCommon(FogEnd, FogDensity);
    glUniform1i(ExpSquaredFogEnabledLoc, 0);
}


void LightingTechnique::SetExpSquaredFog(float FogEnd, float FogDensity)
{
    SetExpFogCommon(FogEnd, FogDensity);
    glUniform1i(ExpSquaredFogEnabledLoc, 1);
}


void LightingTechnique::SetExpFogCommon(float FogEnd, float FogDensity)
{
    if (FogEnd < 0.0f) {
        printf("Fog end must be positive: %f\n", FogEnd);
        exit(1);
    }

    if (FogDensity < 0.0f) {
        printf("Fog density must be positive: %f\n", FogDensity);
        exit(1);
    }

    glUniform1f(FogStartLoc, -1.0f);
    glUniform1f(LayeredFogTopLoc, -1.0f);
    glUniform1f(FogTimeLoc, -1.0f);

    glUniform1f(FogEndLoc, FogEnd);
    glUniform1f(ExpFogDensityLoc, FogDensity);
}


void LightingTechnique::SetLayeredFog(float FogTop, float FogEnd)
{
    if (FogTop < 0.0f) {
        printf("Fog top must be positive: %f\n", FogTop);
        exit(1);
    }

    if (FogEnd < 0.0f) {
        printf("Fog end must be positive: %f\n", FogEnd);
        exit(1);
    }

    glUniform1f(FogStartLoc, -1.0f);
    glUniform1f(FogTimeLoc, -1.0f);

    glUniform1f(LayeredFogTopLoc, FogTop);
    glUniform1f(FogEndLoc, FogEnd);
}


void LightingTechnique::SetFogColor(const Vector3f& FogColor)
{
    glUniform3f(FogColorLoc, FogColor.r, FogColor.g, FogColor.b);
}


void LightingTechnique::SetFogTime(float Time)
{
    glUniform1f(FogTimeLoc, Time);
}


void LightingTechnique::SetAnimatedFog(float FogEnd, float FogDensity)
{
    glUniform1f(FogStartLoc, -1.0f);
    glUniform1f(LayeredFogTopLoc, -1.0f);

    glUniform1f(FogEndLoc, FogEnd);
    glUniform1f(ExpFogDensityLoc, FogDensity);
}


void LightingTechnique::SetPBR(bool IsPBR)
{
    glUniform1i(IsPBRLoc, IsPBR);
}


void LightingTechnique::SetPBRMaterial(const PBRMaterial& Material)
{
    glUniform1f(PBRMaterialLoc.Roughness, Material.Roughness);
    glUniform1i(PBRMaterialLoc.IsMetal, Material.IsMetal);
    glUniform3f(PBRMaterialLoc.Color, Material.Color.r, Material.Color.g, Material.Color.b);
}


void LightingTechnique::SetClipPlane(const Vector3f& Normal, const Vector3f& PointOnPlane)
{
    float d = -Normal.Dot(PointOnPlane);
    glUniform4f(ClipPlaneLoc, Normal.x, Normal.y, Normal.z, d);
}

