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


#version 460 core

#extension GL_ARB_bindless_texture : require

const int MAX_POINT_LIGHTS = 2;
const int MAX_SPOT_LIGHTS = 2;

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 WorldPos0;
in vec4 LightSpacePos0;
in vec3 Tangent0;
in vec3 Bitangent0;
in flat int MaterialIndex;

out vec4 out_FragColor;

vec2 TexCoord;

struct BaseLight
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

struct DirectionalLight
{
    BaseLight Base;
    vec3 Direction;
};

struct Attenuation
{
    float Constant;
    float Linear;
    float Exp;
};

struct PointLight
{
    BaseLight Base;
    vec3 WorldPos;
    Attenuation Atten;
};

struct SpotLight
{
    PointLight Base;
    vec3 Direction;
    float Cutoff;
};

struct MaterialColor
{
    vec4 AmbientColor;
    vec4 DiffuseColor;
    vec4 SpecularColor;
};


layout(std430, binding = 2) readonly buffer ColorSSBO {
    MaterialColor Colors[];
};

layout(std430, binding = 3) readonly buffer DiffuseSSBO {
    sampler2D DiffuseMaps[];
};

layout(std430, binding = 4) readonly buffer NormalSSBO {
    sampler2D NormalMaps[];
};


struct PBRLight {
    vec4 PosDir;   // if w == 1 position, else direction
    vec3 Intensity;
};


struct PBRMaterial
{
    float Roughness;
    bool IsMetal;
    vec3 Color;
    bool IsAlbedo;
};


uniform DirectionalLight gDirectionalLight;
uniform int gNumPointLights;
uniform PointLight gPointLights[MAX_POINT_LIGHTS];
uniform int gNumSpotLights;
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];
uniform MaterialColor gMaterial;
uniform bool gHasSampler = false;
layout(binding = 0) uniform sampler2D gSampler;
layout(binding = 1) uniform sampler2D gSamplerSpecularExponent;
layout(binding = 2) uniform sampler2D gShadowMap;        // required only for shadow mapping (spot/directional light)
layout(binding = 3) uniform samplerCube gShadowCubeMap;  // required only for shadow mapping (point light)
layout(binding = 4) uniform sampler3D gShadowMapOffsetTexture;
layout(binding = 5) uniform sampler2D gNormalMap;
layout(binding = 6) uniform sampler2D gHeightMap;
layout(binding = 7) uniform sampler2D gAlbedo;
layout(binding = 8) uniform sampler2D gRoughness;
layout(binding = 9) uniform sampler2D gMetallic;
layout(binding = 12) uniform samplerCube gCubemapTexture;
uniform bool gHasNormalMap = false;
uniform bool gHasHeightMap = false;
uniform int gShadowMapWidth = 0;
uniform int gShadowMapHeight = 0;
uniform int gShadowMapFilterSize = 0;
uniform float gShadowMapOffsetTextureSize;
uniform float gShadowMapOffsetFilterSize;
uniform float gShadowMapRandomRadius = 0.0;
uniform vec3 gCameraWorldPos;
uniform vec4 gColorMod = vec4(1);
uniform vec4 gColorAdd = vec4(0);
uniform float gRimLightPower = 2.0;
uniform bool gRimLightEnabled = false;
uniform bool gCellShadingEnabled = false;
uniform bool gEnableSpecularExponent = false;
uniform bool gIsPBR = false;
uniform PBRMaterial gPBRmaterial;
uniform bool gLightingEnabled = true;
uniform bool gShadowsEnabled = true;
uniform bool gIsIndirectRender = false;
uniform bool gRefRefractEnabled = true;
uniform float gReflectionFactor = 1.0;
uniform float gMatToRefRefractFactor = 0.5;
uniform float gETA = 1.0;
uniform float gFresnelPower = 1.0;

// Fog
uniform float gExpFogDensity = 1.0;
uniform bool gExpSquaredFogEnabled = false;
uniform float gLayeredFogTop = -1.0;
uniform float gFogStart = -1.0;
uniform float gFogEnd = -1.0;
uniform float gFogTime = -1.0;
uniform vec3 gFogColor = vec3(0.0, 0.0, 0.0);

const int toon_color_levels = 4;
const float toon_scale_factor = 1.0f / toon_color_levels;


vec4 GetMaterialAmbientColor()
{
    if (gIsIndirectRender) {
        return Colors[MaterialIndex].AmbientColor;
    } else {
        return gMaterial.AmbientColor;
    }
}


vec4 GetMaterialDiffuseColor()
{
    if (gIsIndirectRender) {
        return Colors[MaterialIndex].DiffuseColor;
    } else {
        return gMaterial.DiffuseColor;
    }
}


vec4 GetMaterialSpecularColor()
{
    if (gIsIndirectRender) {
        return Colors[MaterialIndex].SpecularColor;
    } else {
        return gMaterial.SpecularColor;
    }
}


void main()
{
    out_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
 /* InputAttributes tc;
  tc.uv[0] = uv0uv1.xy;
  tc.uv[1] = uv0uv1.zw;

  MetallicRoughnessDataGPU mat = getMaterial(getMaterialId());

  vec4 Kao = sampleAO(tc, mat);
  vec4 Ke  = sampleEmissive(tc, mat);
  vec4 Kd  = sampleAlbedo(tc, mat) * color;
  vec4 mrSample = sampleMetallicRoughness(tc, mat);

  // world-space normal
  vec3 n = normalize(normal);

  vec3 normalSample = sampleNormal(tc, mat).xyz;

  // normal mapping
  n = perturbNormal(n, worldPos, normalSample, getNormalUV(tc, mat));

  if (!gl_FrontFacing) n *= -1.0f;

  PBRInfo pbrInputs = calculatePBRInputsMetallicRoughness(Kd, n, perFrame.drawable.cameraPos.xyz, worldPos, mrSample);

  vec3 specular_color = getIBLRadianceContributionGGX(pbrInputs, 1.0);
  vec3 diffuse_color = getIBLRadianceLambertian(pbrInputs.NdotV, n, pbrInputs.perceptualRoughness, pbrInputs.diffuseColor, pbrInputs.reflectance0, 1.0);
  vec3 color = specular_color + diffuse_color;

  // one hardcoded light source
  vec3 lightPos = vec3(0, 0, -5);
  color += calculatePBRLightContribution( pbrInputs, normalize(lightPos - worldPos), vec3(1.0) );
  // ambient occlusion
  color = color * ( Kao.r < 0.01 ? 1.0 : Kao.r );
  // emissive
  color = pow( Ke.rgb + color, vec3(1.0/2.2) );

  out_FragColor = vec4(color, 1.0);

// Uncomment to debug:
//  out_FragColor = vec4((n + vec3(1.0))*0.5, 1.0);
//  out_FragColor = Kao;
//  out_FragColor = Ke;
//  out_FragColor = Kd;
//  vec2 MeR = mrSample.yz;
//  MeR.x *= getMetallicFactor(mat);
//  MeR.y *= getRoughnessFactor(mat);
//  out_FragColor = vec4(MeR.y,MeR.y,MeR.y, 1.0);
//  out_FragColor = mrSample;*/
}
