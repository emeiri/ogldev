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

#define FAIL_ON_MISSING_LOC


#include "GL/gl_forward_lighting.h"

ForwardLightingTechnique::ForwardLightingTechnique()
{
}

bool ForwardLightingTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Framework/Shaders/GL/forward_lighting.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Framework/Shaders/GL/forward_lighting.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    if (!BaseLightingTechnique::Init()) {
        return false;
    }

    return InitCommon();
}

bool ForwardLightingTechnique::InitCommon()
{
    samplerLoc = GetUniformLocation("gSampler");
    hasSamplerLoc = GetUniformLocation("gHasSampler");
    shadowMapLoc = GetUniformLocation("gShadowMap");
    shadowCubeMapLoc = GetUniformLocation("gShadowCubeMap");
    shadowMapWidthLoc = GetUniformLocation("gShadowMapWidth");
    shadowMapHeightLoc = GetUniformLocation("gShadowMapHeight");
    shadowMapFilterSizeLoc = GetUniformLocation("gShadowMapFilterSize");
    shadowMapOffsetTextureLoc = GetUniformLocation("gShadowMapOffsetTexture");
    NormalMapLoc = GetUniformLocation("gNormalMap");
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
    GET_UNIFORM_AND_CHECK(IsPBRLoc, "gIsPBR");
    GET_UNIFORM_AND_CHECK(PBRMaterialLoc.Roughness, "gPBRmaterial.Roughness");
    GET_UNIFORM_AND_CHECK(PBRMaterialLoc.IsMetal, "gPBRmaterial.IsMetal");
    GET_UNIFORM_AND_CHECK(PBRMaterialLoc.Color, "gPBRmaterial.Color");
    GET_UNIFORM_AND_CHECK(PBRMaterialLoc.IsAlbedo, "gPBRmaterial.IsAlbedo");
    
   // GET_UNIFORM_AND_CHECK(HeightMapLoc, "gHeightMap");
    //GET_UNIFORM_AND_CHECK(HasHeightMapLoc, "gHasHeightMap");
    GET_UNIFORM_AND_CHECK(AlbedoLoc, "gAlbedo");
    GET_UNIFORM_AND_CHECK(RoughnessLoc, "gRoughness");
    GET_UNIFORM_AND_CHECK(MetallicLoc, "gMetallic");
    GET_UNIFORM_AND_CHECK(SkyboxLoc, "gCubemapTexture");
   // GET_UNIFORM_AND_CHECK(AOLoc, "gAO");
   // GET_UNIFORM_AND_CHECK(EmissiveLoc, "gEmissive");
    GET_UNIFORM_AND_CHECK(RefRefractEnabledLoc, "gRefRefractEnabled");
  //  GET_UNIFORM_AND_CHECK(ReflectionFactorLoc, "gReflectionFactor");
    GET_UNIFORM_AND_CHECK(MaterialToRefRefractFactorLoc, "gMatToRefRefractFactor");
    GET_UNIFORM_AND_CHECK(ETALoc, "gETA");
    GET_UNIFORM_AND_CHECK(FresnelPowerLoc, "gFresnelPower");

    if (samplerLoc == INVALID_UNIFORM_LOCATION ||
        shadowMapLoc == INVALID_UNIFORM_LOCATION ||
        shadowCubeMapLoc == INVALID_UNIFORM_LOCATION ||
        NormalMapLoc == INVALID_UNIFORM_LOCATION ||
        shadowMapWidthLoc == INVALID_UNIFORM_LOCATION ||
        shadowMapHeightLoc == INVALID_UNIFORM_LOCATION ||
        shadowMapFilterSizeLoc == INVALID_UNIFORM_LOCATION ||
        shadowMapOffsetTextureLoc == INVALID_UNIFORM_LOCATION ||
        ShadowMapOffsetTextureSizeLoc == INVALID_UNIFORM_LOCATION ||
        ShadowMapOffsetFilterSizeLoc == INVALID_UNIFORM_LOCATION ||
        ShadowMapRandomRadiusLoc == INVALID_UNIFORM_LOCATION ||
        samplerSpecularExponentLoc == INVALID_UNIFORM_LOCATION ||
        materialLoc.AmbientColor == INVALID_UNIFORM_LOCATION ||
        materialLoc.DiffuseColor == INVALID_UNIFORM_LOCATION ||
        materialLoc.SpecularColor == INVALID_UNIFORM_LOCATION ||
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
        ColorAddLocation == INVALID_UNIFORM_LOCATION) {
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

        SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.WorldPos", i);
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


void ForwardLightingTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(samplerLoc, TextureUnit);
}


void ForwardLightingTechnique::SetNormalMapTextureUnit(int TextureUnit)
{
    glUniform1i(NormalMapLoc, TextureUnit);
}


void ForwardLightingTechnique::SetHeightMapTextureUnit(int TextureUnit)
{
    glUniform1i(HeightMapLoc, TextureUnit);
}


void ForwardLightingTechnique::SetSkyboxTextureUnit(int TextureUnit)
{
    glUniform1i(SkyboxLoc, TextureUnit);
}


void ForwardLightingTechnique::ControlDiffuseTexture(bool Enable)
{
    glUniform1i(hasSamplerLoc, Enable);
}

void ForwardLightingTechnique::SetShadowMapSize(unsigned int Width, unsigned int Height)
{
    glUniform1i(shadowMapWidthLoc, Width);
    glUniform1i(shadowMapHeightLoc, Height);
}


void ForwardLightingTechnique::SetShadowMapFilterSize(unsigned int Size)
{
    glUniform1i(shadowMapFilterSizeLoc, Size);
}


void ForwardLightingTechnique::SetShadowMapTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(shadowMapLoc, TextureUnit);
}


void ForwardLightingTechnique::SetShadowCubeMapTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(shadowCubeMapLoc, TextureUnit);
}


void ForwardLightingTechnique::SetShadowMapOffsetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(shadowMapOffsetTextureLoc, TextureUnit);
}


void ForwardLightingTechnique::SetShadowMapOffsetTextureParams(float TextureSize, float FilterSize, float Radius)
{
    glUniform1f(ShadowMapOffsetTextureSizeLoc, TextureSize);
    glUniform1f(ShadowMapOffsetFilterSizeLoc, FilterSize);
    glUniform1f(ShadowMapRandomRadiusLoc, Radius);
}


void ForwardLightingTechnique::SetSpecularExponentTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(samplerSpecularExponentLoc, TextureUnit);
}


void ForwardLightingTechnique::SetDirectionalLight(const DirectionalLight& DirLight, bool WithDir)
{
    glUniform3f(dirLightLoc.Color, DirLight.Color.x, DirLight.Color.y, DirLight.Color.z);
    glUniform1f(dirLightLoc.AmbientIntensity, DirLight.AmbientIntensity);
    glUniform1f(dirLightLoc.DiffuseIntensity, DirLight.DiffuseIntensity);

    if (WithDir) {
        UpdateDirLightDirection(DirLight);
    }
}


void ForwardLightingTechnique::UpdateDirLightDirection(const DirectionalLight& DirLight)
{
    Vector3f LocalDirection = DirLight.WorldDirection;

    LocalDirection.Normalize();

    glUniform3f(dirLightLoc.Direction, LocalDirection.x, LocalDirection.y, LocalDirection.z);
}


void ForwardLightingTechnique::SetMaterial(const Material& material)
{
    glUniform4f(materialLoc.AmbientColor, material.AmbientColor.r, material.AmbientColor.g, material.AmbientColor.b, material.AmbientColor.a);
    glUniform4f(materialLoc.DiffuseColor, material.DiffuseColor.r, material.DiffuseColor.g, material.DiffuseColor.b, material.DiffuseColor.a);
    glUniform4f(materialLoc.SpecularColor, material.SpecularColor.r, material.SpecularColor.g, material.SpecularColor.b, material.SpecularColor.a);

    bool HasDiffuseTexture = (material.pTextures[TEX_TYPE_BASE] != NULL);
    ControlDiffuseTexture(HasDiffuseTexture);
}


void ForwardLightingTechnique::SetPointLights(unsigned int NumLights, const PointLight* pLights, bool WithPos)
{
    glUniform1i(NumPointLightsLoc, NumLights);

    for (unsigned int i = 0 ; i < NumLights ; i++) {
        glUniform3f(PointLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(PointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(PointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform1f(PointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(PointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
        // TODO: assimp puts a very small fraction here leading to burnout of the image
        glUniform1f(PointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp * 2000.0f); 
    }

    if (WithPos) {
        UpdatePointLightsPos(NumLights, pLights);
    }
}


void ForwardLightingTechnique::UpdatePointLightsPos(unsigned int NumLights, const PointLight* pLights)
{
    for (unsigned int i = 0 ; i < NumLights ; i++) {
        const Vector3f& WorldPos = pLights[i].WorldPosition;
        glUniform3f(PointLightsLocation[i].WorldPos, WorldPos.x, WorldPos.y, WorldPos.z);
    }
}

void ForwardLightingTechnique::SetSpotLights(unsigned int NumLights, const SpotLight* pLights, bool WithPosAndDir)
{
    glUniform1i(NumSpotLightsLoc, NumLights);

    for (unsigned int i = 0 ; i < NumLights ; i++) {
        glUniform3f(SpotLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(SpotLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(SpotLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform1f(SpotLightsLocation[i].Cutoff, cosf(ToRadian(pLights[i].Cutoff)));
        glUniform1f(SpotLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(SpotLightsLocation[i].Atten.Linear,   pLights[i].Attenuation.Linear);
        // TODO: assimp puts a very small fraction here leading to burnout of the image
        glUniform1f(SpotLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
    }

    if (WithPosAndDir) {
        UpdateSpotLightsPosAndDir(NumLights, pLights);
    }
}


void ForwardLightingTechnique::UpdateSpotLightsPosAndDir(unsigned int NumLights, const SpotLight* pLights)
{
    for (unsigned int i = 0 ; i < NumLights ; i++) {
        Vector3f Direction = pLights[i].WorldDirection;
        Direction.Normalize();
        glUniform3f(SpotLightsLocation[i].Direction, Direction.x, Direction.y, Direction.z);
        const Vector3f& WorldPos = pLights[i].WorldPosition;
        glUniform3f(SpotLightsLocation[i].Position, WorldPos.x, WorldPos.y, WorldPos.z);
    }
}


void ForwardLightingTechnique::SetColorMod(const Vector4f& Color)
{
    glUniform4f(ColorModLocation, Color.x, Color.y, Color.z, Color.w);
}


void ForwardLightingTechnique::SetColorAdd(const Vector4f& Color)
{
    glUniform4f(ColorAddLocation, Color.x, Color.y, Color.z, Color.w);
}


void ForwardLightingTechnique::ControlRimLight(bool IsEnabled)
{
    if (IsEnabled) {
        glUniform1i(EnableRimLightLoc, 1);
    } else {
        glUniform1i(EnableRimLightLoc, 0);
    }
}


void ForwardLightingTechnique::ControlCellShading(bool IsEnabled)
{
    if (IsEnabled) {
        glUniform1i(EnableCellShadingLoc, 1);
    } else {
        glUniform1i(EnableCellShadingLoc, 0);
    }
}


void ForwardLightingTechnique::ControlSpecularExponent(bool IsEnabled)
{
    if (IsEnabled) {
        glUniform1i(EnableSpecularExponent, 1);
    } else {
        glUniform1i(EnableSpecularExponent, 0);
    }
}


void ForwardLightingTechnique::SetLinearFog(float FogStart, float FogEnd)
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


void ForwardLightingTechnique::SetExpFog(float FogEnd, float FogDensity)
{
    SetExpFogCommon(FogEnd, FogDensity);
    glUniform1i(ExpSquaredFogEnabledLoc, 0);
}


void ForwardLightingTechnique::SetExpSquaredFog(float FogEnd, float FogDensity)
{
    SetExpFogCommon(FogEnd, FogDensity);
    glUniform1i(ExpSquaredFogEnabledLoc, 1);
}


void ForwardLightingTechnique::SetExpFogCommon(float FogEnd, float FogDensity)
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


void ForwardLightingTechnique::SetLayeredFog(float FogTop, float FogEnd)
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


void ForwardLightingTechnique::SetFogColor(const Vector3f& FogColor)
{
    glUniform3f(FogColorLoc, FogColor.r, FogColor.g, FogColor.b);
}


void ForwardLightingTechnique::SetFogTime(float Time)
{
    glUniform1f(FogTimeLoc, Time);
}


void ForwardLightingTechnique::SetAnimatedFog(float FogEnd, float FogDensity)
{
    glUniform1f(FogStartLoc, -1.0f);
    glUniform1f(LayeredFogTopLoc, -1.0f);

    glUniform1f(FogEndLoc, FogEnd);
    glUniform1f(ExpFogDensityLoc, FogDensity);
}


void ForwardLightingTechnique::SetPBR(bool IsPBR)
{
    glUniform1i(IsPBRLoc, IsPBR);
}


void ForwardLightingTechnique::SetPBRMaterial(const PBRMaterial& Material)
{
    glUniform1i(PBRMaterialLoc.IsMetal, Material.IsMetal);
    if (Material.pAlbedo) {
        glUniform1i(PBRMaterialLoc.IsAlbedo, 1);
    } else {
        glUniform1i(PBRMaterialLoc.IsAlbedo, 0);
	    glUniform3f(PBRMaterialLoc.Color, Material.Color.r, Material.Color.g, Material.Color.b);
        glUniform1f(PBRMaterialLoc.Roughness, Material.Roughness);
	}
}


void ForwardLightingTechnique::ControlRefRefract(bool Enable)
{
    glUniform1i(RefRefractEnabledLoc, Enable);
}


void ForwardLightingTechnique::SetReflectionFactor(float f)
{
    glUniform1f(ReflectionFactorLoc, f);
}


void ForwardLightingTechnique::SetMaterialToRefRefractFactor(float f)
{
    //printf("%f\n", f);
    glUniform1f(MaterialToRefRefractFactorLoc, f);
}


void ForwardLightingTechnique::SetRefractETA(float eta)
{
    glUniform1f(ETALoc, eta);
}


void ForwardLightingTechnique::SetFresnelPower(float f)
{
    glUniform1f(FresnelPowerLoc, f);
}


void ForwardLightingTechnique::SetAlbedoTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(AlbedoLoc, TextureUnit);
}


void ForwardLightingTechnique::SetRoughnessTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(RoughnessLoc, TextureUnit);
}


void ForwardLightingTechnique::SetMetallicTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(MetallicLoc, TextureUnit);
}


void ForwardLightingTechnique::SetAOTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(AOLoc, TextureUnit);
}


void ForwardLightingTechnique::SetEmissiveTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(EmissiveLoc, TextureUnit);
}
