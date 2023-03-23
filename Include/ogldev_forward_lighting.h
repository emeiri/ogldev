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

#ifndef FORWARD_LIGHTING_TECHNIQUE_H
#define FORWARD_LIGHTING_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"
#include "ogldev_material.h"
#include "ogldev_world_transform.h"
#include "ogldev_mesh_common.h"

// This version of the lighting technique was updated for youtube

class BaseLight
{
public:
    Vector3f Color = Vector3f(1.0f, 1.0f, 1.0f);
    float AmbientIntensity = 0.0f;
    float DiffuseIntensity = 0.0f;

    bool IsZero() const
    {
        return ((AmbientIntensity == 0) && (DiffuseIntensity == 0.0f));
    }
};


class DirectionalLight : public BaseLight
{
public:
    Vector3f WorldDirection = Vector3f(0.0f, 0.0f, 0.0f);
};


struct LightAttenuation
{
    float Constant = 1.0f;
    float Linear = 0.0f;
    float Exp = 0.0f;
};


class PointLight: public BaseLight
{
public:
    Vector3f WorldPosition = Vector3f(0.0f, 0.0f, 0.0f);
    LightAttenuation Attenuation;
};


class SpotLight : public PointLight
{
public:
    Vector3f WorldDirection = Vector3f(0.0f, 0.0f, 0.0f);
    float Cutoff = 0.0f;
};


class ForwardLightingTechnique : public Technique, public IRenderCallbacks
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
    void DisableDiffuseTexture();
    void SetShadowMapTextureUnit(unsigned int TextureUnit);
    void SetShadowCubeMapTextureUnit(unsigned int TextureUnit);
    void SetShadowMapSize(unsigned int Width, unsigned int Height);
    void SetShadowMapFilterSize(unsigned int Size);
    void SetShadowMapOffsetTextureUnit(unsigned int TextureUnit);
    void SetShadowMapOffsetTextureParams(float TextureSize, float FilterSize, float Radius);
    void SetSpecularExponentTextureUnit(unsigned int TextureUnit);
    void SetDirectionalLight(const DirectionalLight& DirLight, bool WithDir = true);
    void UpdateDirLightDirection(const DirectionalLight& DirLight);
    void SetPointLights(unsigned int NumLights, const PointLight* pLights, bool WithPos = true);
    void UpdatePointLight(unsigned int Index, const PointLight& Light);
    void UpdatePointLightsPos(unsigned int NumLights, const PointLight* pLights);
    void SetSpotLights(unsigned int NumLights, const SpotLight* pLights, bool WithPosAndDir = true);
    void UpdateSpotLight(unsigned int Index, const SpotLight& Light);
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

protected:

    bool InitCommon();

private:
    void SetExpFogCommon(float FogEnd, float FogDensity);

    GLuint WVPLoc = INVALID_UNIFORM_LOCATION;
    GLuint WorldMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint NormalMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint LightWVPLoc = INVALID_UNIFORM_LOCATION; // required only for shadow mapping
    GLuint samplerLoc = INVALID_UNIFORM_LOCATION;
    GLuint hasSamplerLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowCubeMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapWidthLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapHeightLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapFilterSizeLoc = INVALID_UNIFORM_LOCATION;
    GLuint ShadowMapOffsetTextureLoc = INVALID_UNIFORM_LOCATION;
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

    struct {
        GLuint AmbientColor;
        GLuint DiffuseColor;
        GLuint SpecularColor;
    } materialLoc;

    struct {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint Direction;
        GLuint DiffuseIntensity;
    } dirLightLoc;

    struct {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint WorldPos;
        GLuint DiffuseIntensity;

        struct
        {
            GLuint Constant;
            GLuint Linear;
            GLuint Exp;
        } Atten;
    } PointLightsLocation[MAX_POINT_LIGHTS];

    struct {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint DiffuseIntensity;
        GLuint Position;
        GLuint Direction;
        GLuint Cutoff;
        struct {
            GLuint Constant;
            GLuint Linear;
            GLuint Exp;
        } Atten;
    } SpotLightsLocation[MAX_SPOT_LIGHTS];
};


#endif  /* FORWARD_LIGHTING_TECHNIQUE_H */
