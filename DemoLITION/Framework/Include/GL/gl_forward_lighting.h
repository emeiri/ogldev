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

#pragma once

#include "technique.h"
#include "ogldev_math_3d.h"
#include "ogldev_material.h"
#include "ogldev_world_transform.h"
#include "demolition_lights.h"
#include "Int/core_model.h"
#include "GL/gl_base_lighting_technique.h"

class ForwardLightingTechnique : public BaseLightingTechnique
{
public:

    ForwardLightingTechnique();

    virtual bool Init();

    void SetTextureUnit(unsigned int TextureUnit);
    void SetShadowMapTextureUnit(unsigned int TextureUnit);
    void SetShadowCubeMapTextureUnit(unsigned int TextureUnit);
    void SetShadowMapSize(unsigned int Width, unsigned int Height);
    void SetShadowMapFilterSize(unsigned int Size);
    void SetShadowMapOffsetTextureUnit(unsigned int TextureUnit);
    void SetShadowMapOffsetTextureParams(float TextureSize, float FilterSize, float Radius);
    void SetSpecularExponentTextureUnit(unsigned int TextureUnit);
    void SetAlbedoTextureUnit(unsigned int TextureUnit);
    void SetRoughnessTextureUnit(unsigned int TextureUnit);
    void SetMetallicTextureUnit(unsigned int TextureUnit);
    void SetAOTextureUnit(unsigned int TextureUnit);
    void SetEmissiveTextureUnit(unsigned int TextureUnit);
    void SetNormalMapTextureUnit(int TextureUnit);
    void SetHeightMapTextureUnit(int TextureUnit);
    void SetSkyboxTextureUnit(int TextureUnit);
    void SetDirectionalLight(const DirectionalLight& DirLight, bool WithDir = true);
    void UpdateDirLightDirection(const DirectionalLight& DirLight);
    void UpdateSpotLightsPosAndDir(unsigned int NumLights, const SpotLight* pLights);
    virtual void SetMaterial(const Material& material);
    void SetColorMod(const Vector4f& ColorMod);
    void SetColorAdd(const Vector4f& ColorAdd);
    void ControlRimLight(bool IsEnabled);
    void ControlCellShading(bool IsEnabled);
    virtual void ControlSpecularExponent(bool IsEnabled);
    void SetLinearFog(float FogStart, float FogEnd);
    void SetExpFog(float FogEnd, float FogDensity);
    void SetExpSquaredFog(float FogEnd, float FogDensity);
    void SetLayeredFog(float FogTop, float FogEnd);
    void SetFogColor(const Vector3f& FogColor);
    void SetAnimatedFog(float FogEnd, float FogDensity);
    void SetFogTime(float Time);
    void SetPBR(bool IsPBR);
    void SetPBRMaterial(const PBRMaterial& Material);
    void ControlRefRefract(bool Enable);
    void SetReflectionFactor(float f);
    void SetMaterialToRefRefractFactor(float f);
    void SetRefractETA(float f);
    void SetFresnelPower(float f);

protected:

    bool InitCommon();

private:
    void SetExpFogCommon(float FogEnd, float FogDensity);
    void ControlDiffuseTexture(bool Enable);

    GLuint samplerLoc = INVALID_UNIFORM_LOCATION;
    GLuint hasSamplerLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowCubeMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint NormalMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint HeightMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapWidthLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapHeightLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapFilterSizeLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapOffsetTextureLoc = INVALID_UNIFORM_LOCATION;
    GLuint ShadowMapOffsetTextureSizeLoc = INVALID_UNIFORM_LOCATION;
    GLuint ShadowMapOffsetFilterSizeLoc = INVALID_UNIFORM_LOCATION;
    GLuint ShadowMapRandomRadiusLoc = INVALID_UNIFORM_LOCATION;
    GLuint samplerSpecularExponentLoc = INVALID_UNIFORM_LOCATION;
    GLuint NumPointLightsLoc = INVALID_UNIFORM_LOCATION;
    GLuint NumSpotLightsLoc = INVALID_UNIFORM_LOCATION;
    GLuint ColorModLocation = INVALID_UNIFORM_LOCATION;
    GLuint ColorAddLocation = INVALID_UNIFORM_LOCATION;
    GLuint EnableRimLightLoc = INVALID_UNIFORM_LOCATION;
    GLuint EnableCellShadingLoc = INVALID_UNIFORM_LOCATION;
    GLuint EnableSpecularExponent = INVALID_UNIFORM_LOCATION;
    GLuint FogStartLoc = INVALID_UNIFORM_LOCATION;
    GLuint FogEndLoc = INVALID_UNIFORM_LOCATION;
    GLuint FogColorLoc = INVALID_UNIFORM_LOCATION;
    GLuint ExpFogDensityLoc = INVALID_UNIFORM_LOCATION;
    GLuint ExpSquaredFogEnabledLoc = INVALID_UNIFORM_LOCATION;
    GLuint LayeredFogTopLoc = INVALID_UNIFORM_LOCATION;
    GLuint FogTimeLoc = INVALID_UNIFORM_LOCATION;
    GLuint IsPBRLoc = INVALID_UNIFORM_LOCATION;
    GLuint AlbedoLoc = INVALID_UNIFORM_LOCATION;
    GLuint RoughnessLoc = INVALID_UNIFORM_LOCATION;
    GLuint MetallicLoc = INVALID_UNIFORM_LOCATION;
    GLuint AOLoc = INVALID_UNIFORM_LOCATION;
    GLuint EmissiveLoc = INVALID_UNIFORM_LOCATION;
    GLuint SkyboxLoc = INVALID_UNIFORM_LOCATION;
    GLuint RefRefractEnabledLoc = INVALID_UNIFORM_LOCATION;
    GLuint ReflectionFactorLoc = INVALID_UNIFORM_LOCATION;
    GLuint MaterialToRefRefractFactorLoc = INVALID_UNIFORM_LOCATION;
    GLuint ETALoc = INVALID_UNIFORM_LOCATION;
    GLuint FresnelPowerLoc = INVALID_UNIFORM_LOCATION;

    struct {
        GLuint AmbientColor = INVALID_UNIFORM_LOCATION;
        GLuint DiffuseColor = INVALID_UNIFORM_LOCATION;
        GLuint SpecularColor = INVALID_UNIFORM_LOCATION;
    } materialLoc;

    struct {
        GLuint Roughness;
        GLuint IsMetal;
        GLuint Color;
        GLuint IsAlbedo;
    } PBRMaterialLoc;
};

