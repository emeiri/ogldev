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

class ForwardLightingTechnique : public Technique
{
public:

    static const unsigned int MAX_POINT_LIGHTS = 2;
    static const unsigned int MAX_SPOT_LIGHTS = 2;

    ForwardLightingTechnique();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);
    void SetWorldMatrix(const Matrix4f& WVP);
    void SetNormalMatrix(const Matrix3f& NormalMatrix);
    void SetLightWVP(const Matrix4f& LightWVP); // required only for shadow mapping
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
    void ControlNormalMap(bool Enable);
    void ControlParallaxMap(bool Enable);
    void SetDirectionalLight(const DirectionalLight& DirLight, bool WithDir = true);
    void UpdateDirLightDirection(const DirectionalLight& DirLight);
    void SetPointLights(unsigned int NumLights, const PointLight* pLights, bool WithPos = true);
    void UpdatePointLightsPos(unsigned int NumLights, const PointLight* pLights);
    void SetSpotLights(unsigned int NumLights, const SpotLight* pLights, bool WithPosAndDir = true);
    void UpdateSpotLightsPosAndDir(unsigned int NumLights, const SpotLight* pLights);
    void SetCameraWorldPos(const Vector3f& CameraWorldPos);
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
    void ControlLighting(bool LightingEnabled);
    void ControlShadows(bool ShadowsEnabled);
    void ControlIndirectRender(bool IsRenderIndirect);
    void ControlPVP(bool IsPVP);
    void SetVP(const Matrix4f& VP);
    void SetLightVP(const Matrix4f& LightVP);
    void SetPBR(bool IsPBR);
    void SetPBRMaterial(const PBRMaterial& Material);
    void ControlRefRefract(bool Enable);
    void SetReflectionFactor(float f);
    void SetMaterialToRefRefractFactor(float f);
    void SetIndexOfRefraction(float f);

protected:

    bool InitCommon();

private:
    void SetExpFogCommon(float FogEnd, float FogDensity);
    void ControlDiffuseTexture(bool Enable);

    GLuint WVPLoc = INVALID_UNIFORM_LOCATION;
    GLuint WorldMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint NormalMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint LightWVPLoc = INVALID_UNIFORM_LOCATION; // required only for shadow mapping
    GLuint LightVPLoc = INVALID_UNIFORM_LOCATION;  // required only for shadow mapping with indirect rendering
    GLuint samplerLoc = INVALID_UNIFORM_LOCATION;
    GLuint hasSamplerLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowCubeMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint NormalMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint HasNormalMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint HeightMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint HasHeightMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapWidthLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapHeightLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapFilterSizeLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapOffsetTextureLoc = INVALID_UNIFORM_LOCATION;
    GLuint ShadowMapOffsetTextureSizeLoc = INVALID_UNIFORM_LOCATION;
    GLuint ShadowMapOffsetFilterSizeLoc = INVALID_UNIFORM_LOCATION;
    GLuint ShadowMapRandomRadiusLoc = INVALID_UNIFORM_LOCATION;
    GLuint samplerSpecularExponentLoc = INVALID_UNIFORM_LOCATION;
    GLuint CameraWorldPosLoc = INVALID_UNIFORM_LOCATION;
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
    GLuint LightingEnabledLoc = INVALID_UNIFORM_LOCATION;
    GLuint ShadowsEnabledLoc = INVALID_UNIFORM_LOCATION;
    GLuint IsIndirectRenderLoc = INVALID_UNIFORM_LOCATION;
    GLuint IsPVPLoc = INVALID_UNIFORM_LOCATION;
    GLuint VPLoc = INVALID_UNIFORM_LOCATION;
    GLuint AlbedoLoc = INVALID_UNIFORM_LOCATION;
    GLuint RoughnessLoc = INVALID_UNIFORM_LOCATION;
    GLuint MetallicLoc = INVALID_UNIFORM_LOCATION;
    GLuint AOLoc = INVALID_UNIFORM_LOCATION;
    GLuint EmissiveLoc = INVALID_UNIFORM_LOCATION;
    GLuint SkyboxLoc = INVALID_UNIFORM_LOCATION;
    GLuint RefRefractEnabledLoc = INVALID_UNIFORM_LOCATION;
    GLuint ReflectionFactorLoc = INVALID_UNIFORM_LOCATION;
    GLuint MaterialToRefRefractFactorLoc = INVALID_UNIFORM_LOCATION;
    GLuint IndexOfRefractionLoc = INVALID_UNIFORM_LOCATION;

    struct {
        GLuint AmbientColor = INVALID_UNIFORM_LOCATION;
        GLuint DiffuseColor = INVALID_UNIFORM_LOCATION;
        GLuint SpecularColor = INVALID_UNIFORM_LOCATION;
    } materialLoc;

    struct {
        GLuint Color = INVALID_UNIFORM_LOCATION;
        GLuint AmbientIntensity = INVALID_UNIFORM_LOCATION;
        GLuint Direction = INVALID_UNIFORM_LOCATION;
        GLuint DiffuseIntensity = INVALID_UNIFORM_LOCATION;
    } dirLightLoc;

    struct {
        GLuint Color = INVALID_UNIFORM_LOCATION;
        GLuint AmbientIntensity = INVALID_UNIFORM_LOCATION;
        GLuint WorldPos = INVALID_UNIFORM_LOCATION;
        GLuint DiffuseIntensity = INVALID_UNIFORM_LOCATION;

        struct
        {
            GLuint Constant = INVALID_UNIFORM_LOCATION;
            GLuint Linear = INVALID_UNIFORM_LOCATION;
            GLuint Exp = INVALID_UNIFORM_LOCATION;
        } Atten;
    } PointLightsLocation[MAX_POINT_LIGHTS];

    struct {
        GLuint Color = INVALID_UNIFORM_LOCATION;
        GLuint AmbientIntensity = INVALID_UNIFORM_LOCATION;
        GLuint DiffuseIntensity = INVALID_UNIFORM_LOCATION;
        GLuint Position = INVALID_UNIFORM_LOCATION;
        GLuint Direction = INVALID_UNIFORM_LOCATION;
        GLuint Cutoff = INVALID_UNIFORM_LOCATION;
        struct {
            GLuint Constant = INVALID_UNIFORM_LOCATION;
            GLuint Linear = INVALID_UNIFORM_LOCATION;
            GLuint Exp = INVALID_UNIFORM_LOCATION;
        } Atten;
    } SpotLightsLocation[MAX_SPOT_LIGHTS];
    struct {
        GLuint Roughness;
        GLuint IsMetal;
        GLuint Color;
        GLuint IsAlbedo;
    } PBRMaterialLoc;
};

