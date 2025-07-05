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
#extension GL_KHR_vulkan_glsl : require
#extension GL_NV_uniform_buffer_std430_layout : require

const int MAX_POINT_LIGHTS = 2;
const int MAX_SPOT_LIGHTS = 2;

in vec2 TexCoord0;
in vec2 TexCoord1;
in vec3 Normal0;
in vec3 WorldPos0;
in vec4 LightSpacePos0;
in vec3 Tangent0;
in vec3 Bitangent0;
in flat int MaterialIndex;
in vec4 Color0;

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


uniform DirectionalLight gDirectionalLight;
uniform int gNumPointLights;
uniform PointLight gPointLights[MAX_POINT_LIGHTS];
uniform int gNumSpotLights;
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];
uniform bool gHasSampler = false;
layout(binding = 0) uniform sampler2D gAlbedo;
layout(binding = 1) uniform sampler2D gSamplerSpecularExponent;
layout(binding = 2) uniform sampler2D gShadowMap;        // required only for shadow mapping (spot/directional light)
layout(binding = 3) uniform samplerCube gShadowCubeMap;  // required only for shadow mapping (point light)
layout(binding = 4) uniform sampler3D gShadowMapOffsetTexture;
layout(binding = 5) uniform sampler2D gNormalMap;
layout(binding = 6) uniform sampler2D gHeightMap;
layout(binding = 7) uniform sampler2D gBRDF_LUT;
layout(binding = 8) uniform sampler2D gRoughness;
layout(binding = 9) uniform sampler2D gMetallic;
layout(binding = 10) uniform sampler2D gAmbientOcclusion;
layout(binding = 11) uniform sampler2D gEmissive;
layout(binding = 12) uniform samplerCube gCubemapTexture;
layout(binding = 13) uniform samplerCube gEnvMap;
layout(binding = 14) uniform samplerCube gIrradiance;
uniform bool gHasNormalMap = false;
uniform bool gHasHeightMap = false;
uniform int gShadowMapWidth = 0;
uniform int gShadowMapHeight = 0;
uniform int gShadowMapFilterSize = 0;
uniform float gShadowMapOffsetTextureSize;
uniform float gShadowMapOffsetFilterSize;
uniform float gShadowMapRandomRadius = 0.0;
uniform vec3 gCameraWorldPos;
uniform bool gLightingEnabled = true;
uniform bool gShadowsEnabled = true;
uniform bool gIsIndirectRender = false;
uniform vec4 gBaseColor;
uniform vec4 gEmissiveColor;

const float M_PI = 3.141592653589793;

vec4 SRGBtoLINEAR(vec4 srgbIn) 
{
  vec3 linOut = pow(srgbIn.xyz,vec3(2.2));

  return vec4(linOut, srgbIn.a);
}

// http://www.thetenthplanet.de/archives/1180
// modified to fix handedness of the resulting cotangent frame
mat3 cotangentFrame( vec3 N, vec3 p, vec2 uv ) {
  // get edge vectors of the pixel triangle
  vec3 dp1 = dFdx( p );
  vec3 dp2 = dFdy( p );
  vec2 duv1 = dFdx( uv );
  vec2 duv2 = dFdy( uv );

  // solve the linear system
  vec3 dp2perp = cross( dp2, N );
  vec3 dp1perp = cross( N, dp1 );
  vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  // construct a scale-invariant frame
  float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );

  // calculate handedness of the resulting cotangent frame
  float w = (dot(cross(N, T), B) < 0.0) ? -1.0 : 1.0;

  // adjust tangent if needed
  T = T * w;

  return mat3( T * invmax, B * invmax, N );
}

vec3 perturbNormal(vec3 n, vec3 v, vec3 normalSample, vec2 uv) 
{
    vec3 map = normalize( 2.0 * normalSample - vec3(1.0) );
    mat3 TBN = cotangentFrame(n, v, uv);
    return normalize(TBN * map);
}


struct InputAttributes {
  vec2 uv[2];
};

// corresponds to MetallicRoughnessDataGPU from Chapter06/04_MetallicRoughness/src/main.cpp
struct MetallicRoughnessDataGPU {
  vec4 baseColorFactor;
  vec4 metallicRoughnessNormalOcclusion; // packed metallicFactor, roughnessFactor, normalScale, occlusionStrength
  vec4 emissiveFactorAlphaCutoff;        // packed vec3 emissiveFactor + float AlphaCutoff
  uint occlusionTexture;
  sampler2D occlusionTextureSampler;
  uint occlusionTextureUV;
  uint emissiveTexture;
  sampler2D emissiveTextureSampler;
  uint emissiveTextureUV;
  uint baseColorTexture;
  sampler2D baseColorTextureSampler;
  uint baseColorTextureUV;
  uint metallicRoughnessTexture;
  sampler2D metallicRoughnessTextureSampler;
  uint metallicRoughnessTextureUV;
  uint normalTexture;
  sampler2D normalTextureSampler;
  uint normalTextureUV;
  uint alphaMode;
};

// corresponds to EnvironmentMapDataGPU from shared/UtilsGLTF.h 
struct EnvironmentMapDataGPU {
  uint envMapTexture;
  samplerCube envMapTextureSampler;
  uint envMapTextureIrradiance;
  samplerCube envMapTextureIrradianceSampler;
  uint texBRDF_LUT;
  sampler2D texBRDF_LUTSampler;
  uint unused0;
  uint unused1;
};

layout(std430, binding = 2) readonly buffer Materials {
  MetallicRoughnessDataGPU material[];
};

layout(std430, binding = 3) readonly buffer Environments {
  EnvironmentMapDataGPU environment[];
};

uint getEnvironmentId() 
{
    return 0;
}


MetallicRoughnessDataGPU GetMaterial() 
{
    if (gIsIndirectRender) {
        return material[MaterialIndex]; 
    } else {
        MetallicRoughnessDataGPU ret;
        ret.occlusionTextureUV = 0;
        ret.occlusionTextureSampler = gAmbientOcclusion;
        ret.emissiveTextureUV = 0;
        ret.emissiveFactorAlphaCutoff = gEmissiveColor;
        ret.emissiveTextureSampler = gEmissive;
        ret.baseColorTextureUV = 0;
        ret.baseColorTextureSampler = gAlbedo;
        ret.baseColorFactor = gBaseColor;                
        ret.metallicRoughnessTextureSampler = gRoughness;
        ret.metallicRoughnessTextureUV = 0;
        ret.normalTextureSampler = gNormalMap;
        ret.normalTextureUV = 0;
        ret.metallicRoughnessNormalOcclusion = vec4(1.0);// TODO: get from Assimp
        return ret;
    }      
}

EnvironmentMapDataGPU getEnvironment(uint idx) 
{
    if (gIsIndirectRender) {
        return environment[idx]; 
    } else {
        EnvironmentMapDataGPU ret;
        //ret.envMapTexture = ;
        ret.envMapTextureSampler = gEnvMap;
        //ret.envMapTextureIrradiance = ;
        ret.envMapTextureIrradianceSampler = gIrradiance;
        //ret.texBRDF_LUT =;
        ret.texBRDF_LUTSampler = gBRDF_LUT;
        return ret;
    }  
}

float getMetallicFactor(MetallicRoughnessDataGPU mat) {
  return mat.metallicRoughnessNormalOcclusion.x;
}

float getRoughnessFactor(MetallicRoughnessDataGPU mat) {
  return mat.metallicRoughnessNormalOcclusion.y;
}

float getNormalScale(MetallicRoughnessDataGPU mat) {
  return mat.metallicRoughnessNormalOcclusion.z;
}

float getOcclusionFactor(MetallicRoughnessDataGPU mat) {
  return mat.metallicRoughnessNormalOcclusion.w;
}

vec2 getNormalUV(InputAttributes tc, MetallicRoughnessDataGPU mat) {
  return tc.uv[mat.normalTextureUV];
}

vec4 sampleAO(InputAttributes tc, MetallicRoughnessDataGPU mat) {
  return texture(mat.occlusionTextureSampler, tc.uv[mat.occlusionTextureUV]);
}

vec4 sampleEmissive(InputAttributes tc, MetallicRoughnessDataGPU mat) {
  return texture(mat.emissiveTextureSampler, tc.uv[mat.emissiveTextureUV]) * vec4(mat.emissiveFactorAlphaCutoff.xyz, 1.0f);
}

vec4 sampleAlbedo(InputAttributes tc, MetallicRoughnessDataGPU mat) {
  return texture(mat.baseColorTextureSampler, tc.uv[mat.baseColorTextureUV]) * mat.baseColorFactor;
}

vec4 sampleMetallicRoughness(InputAttributes tc, MetallicRoughnessDataGPU mat) {
  return texture(mat.metallicRoughnessTextureSampler, tc.uv[mat.metallicRoughnessTextureUV]);
}

vec4 sampleNormal(InputAttributes tc, MetallicRoughnessDataGPU mat) {
  return texture(mat.normalTextureSampler, tc.uv[mat.normalTextureUV]);
}

vec4 sampleBRDF_LUT(vec2 tc, EnvironmentMapDataGPU map) 
{
    return texture(map.texBRDF_LUTSampler, tc);
}

vec4 sampleEnvMap(vec3 tc, EnvironmentMapDataGPU map) 
{
   // vec3 tc_flip = tc;
   // tc_flip.y = -tc_flip.y;
    return texture(map.envMapTextureSampler, tc);
}

vec4 sampleEnvMapLod(vec3 tc, float lod, EnvironmentMapDataGPU map) 
{
   // vec3 tc_flip = tc;
   // tc_flip.y = -tc_flip.y;
    return textureLod(map.envMapTextureSampler, tc, lod);
}

vec4 sampleEnvMapIrradiance(vec3 tc, EnvironmentMapDataGPU map) 
{
    return texture(map.envMapTextureIrradianceSampler, tc);
}

int sampleEnvMapQueryLevels(EnvironmentMapDataGPU map) {
  return textureQueryLevels(map.envMapTextureSampler);
}


// Based on: https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/pbr.frag

// Encapsulate the various inputs used by the various functions in the shading equation
// We store values in this struct to simplify the integration of alternative implementations
// of the shading terms, outlined in the Readme.MD Appendix.
struct PBRInfo {
  // geometry properties
  float NdotL;                  // cos angle between normal and light direction
  float NdotV;                  // cos angle between normal and view direction
  float NdotH;                  // cos angle between normal and half vector
  float LdotH;                  // cos angle between light direction and half vector
  float VdotH;                  // cos angle between view direction and half vector
  vec3 n;                       // normal at surface point
  vec3 v;                       // vector from surface point to camera

  // material properties
  float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
  vec3 reflectance0;            // full reflectance color (normal incidence angle)
  vec3 reflectance90;           // reflectance color at grazing angle
  float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
  vec3 diffuseColor;            // color contribution from diffuse lighting
  vec3 specularColor;           // color contribution from specular lighting
};

// specularWeight is introduced with KHR_materials_specular
vec3 getIBLRadianceLambertian(float NdotV, vec3 n, float roughness, vec3 diffuseColor, vec3 F0, float specularWeight) {
  vec2 brdfSamplePoint = clamp(vec2(NdotV, roughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
  EnvironmentMapDataGPU envMap = getEnvironment(getEnvironmentId());
  vec2 f_ab = sampleBRDF_LUT(brdfSamplePoint, envMap).rg;

  vec3 irradiance = sampleEnvMapIrradiance(normalize(n.xyz), envMap).rgb;

  //return irradiance;
  // see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
  // Roughness dependent fresnel, from Fdez-Aguera
  vec3 Fr = max(vec3(1.0 - roughness), F0) - F0;
  vec3 k_S = F0 + Fr * pow(1.0 - NdotV, 5.0);
  vec3 FssEss = specularWeight * k_S * f_ab.x + f_ab.y; // <--- GGX / specular light contribution (scale it down if the specularWeight is low)

  // Multiple scattering, from Fdez-Aguera
  float Ems = (1.0 - (f_ab.x + f_ab.y));
  vec3 F_avg = specularWeight * (F0 + (1.0 - F0) / 21.0);
  vec3 FmsEms = Ems * FssEss * F_avg / (1.0 - F_avg * Ems);
  vec3 k_D = diffuseColor * (1.0 - FssEss + FmsEms); // we use +FmsEms as indicated by the formula in the blog post (might be a typo in the implementation)

  return (FmsEms + k_D) * irradiance;
}

// Calculation of the lighting contribution from an optional Image Based Light source.
// Precomputed Environment Maps are required uniform inputs and are computed as outlined in [1].
// See our README.md on Environment Maps [3] for additional discussion.
vec3 getIBLRadianceContributionGGX(PBRInfo pbrInputs, float specularWeight) {
  vec3 n = pbrInputs.n;
  vec3 v =  pbrInputs.v;
  vec3 reflection = -normalize(reflect(v, n));
  EnvironmentMapDataGPU envMap = getEnvironment(getEnvironmentId());
  float mipCount = float(sampleEnvMapQueryLevels(envMap));
  float lod = pbrInputs.perceptualRoughness * (mipCount - 1);

  // retrieve a scale and bias to F0. See [1], Figure 3
  vec2 brdfSamplePoint = clamp(vec2(pbrInputs.NdotV, pbrInputs.perceptualRoughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
  vec3 brdf = sampleBRDF_LUT(brdfSamplePoint, envMap).rgb;
  // HDR envmaps are already linear
  vec3 specularLight = sampleEnvMapLod(reflection.xyz, lod, envMap).rgb;

  //specularLight = texture(gEnvMap, n).rgb;

  //return specularLight;

  // see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
  // Roughness dependent fresnel, from Fdez-Aguera
  vec3 Fr = max(vec3(1.0 - pbrInputs.perceptualRoughness), pbrInputs.reflectance0) - pbrInputs.reflectance0;
  vec3 k_S = pbrInputs.reflectance0 + Fr * pow(1.0 - pbrInputs.NdotV, 5.0);
  vec3 FssEss = k_S * brdf.x + brdf.y;

  return specularWeight * specularLight * FssEss;
}

// Disney Implementation of diffuse from Physically-Based Shading at Disney by Brent Burley. See Section 5.3.
// http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
vec3 diffuseBurley(PBRInfo pbrInputs) {
  float f90 = 2.0 * pbrInputs.LdotH * pbrInputs.LdotH * pbrInputs.alphaRoughness - 0.5;

  return (pbrInputs.diffuseColor / M_PI) * (1.0 + f90 * pow((1.0 - pbrInputs.NdotL), 5.0)) * (1.0 + f90 * pow((1.0 - pbrInputs.NdotV), 5.0));
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs) {
  return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs) {
  float NdotL = pbrInputs.NdotL;
  float NdotV = pbrInputs.NdotV;
  float rSqr = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;

  float attenuationL = 2.0 * NdotL / (NdotL + sqrt(rSqr + (1.0 - rSqr) * (NdotL * NdotL)));
  float attenuationV = 2.0 * NdotV / (NdotV + sqrt(rSqr + (1.0 - rSqr) * (NdotV * NdotV)));
  return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs) {
  float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
  float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
  return roughnessSq / (M_PI * f * f);
}

PBRInfo calculatePBRInputsMetallicRoughness( vec4 albedo, vec3 normal, vec3 cameraPos, vec3 worldPos, vec4 mrSample) {
  PBRInfo pbrInputs;

  MetallicRoughnessDataGPU mat = GetMaterial();
  float perceptualRoughness = getRoughnessFactor(mat);
  float metallic = getMetallicFactor(mat) * mrSample.b;
  metallic = clamp(metallic, 0.0, 1.0);

  // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
  // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
  perceptualRoughness = mrSample.g * perceptualRoughness;
  const float c_MinRoughness = 0.04;

  perceptualRoughness = clamp(perceptualRoughness, c_MinRoughness, 1.0);

  // Roughness is authored as perceptual roughness; as is convention,
  // convert to material roughness by squaring the perceptual roughness [2].
  float alphaRoughness = perceptualRoughness * perceptualRoughness;

  // The albedo may be defined from a base texture or a flat color
  vec4 baseColor = albedo;

  vec3 f0 = vec3(0.04);
  vec3 diffuseColor = mix(baseColor.rgb, vec3(0), metallic); 
  vec3 specularColor = mix(f0, baseColor.rgb, metallic);

  // Compute reflectance.
  float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

  // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
  // For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
  float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
  vec3 specularEnvironmentR0 = specularColor.rgb;
  vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

  vec3 n = normalize(normal);          // normal at surface point
  vec3 v = normalize(cameraPos - worldPos);  // Vector from surface point to camera

  pbrInputs.NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
  pbrInputs.perceptualRoughness = perceptualRoughness;
  pbrInputs.reflectance0 = specularEnvironmentR0;
  pbrInputs.reflectance90 = specularEnvironmentR90;
  pbrInputs.alphaRoughness = alphaRoughness;
  pbrInputs.diffuseColor = diffuseColor;
  pbrInputs.specularColor = specularColor;
  pbrInputs.n = n;
  pbrInputs.v = v;

  return pbrInputs;
}

vec3 calculatePBRLightContribution( inout PBRInfo pbrInputs, vec3 lightDirection, vec3 lightColor ) {
  vec3 n = pbrInputs.n;
  vec3 v = pbrInputs.v;
  vec3 ld = normalize(lightDirection);  // Vector from surface point to light
  vec3 h = normalize(ld+v);        // Half vector between both l and v

  float NdotV = pbrInputs.NdotV;
  float NdotL = clamp(dot(n, ld), 0.001, 1.0);
  float NdotH = clamp(dot(n, h), 0.0, 1.0);
  float LdotH = clamp(dot(ld, h), 0.0, 1.0);
  float VdotH = clamp(dot(v, h), 0.0, 1.0);

  vec3 color = vec3(0);

  if (NdotL > 0.0 || NdotV > 0.0) {
    pbrInputs.NdotL = NdotL;
    pbrInputs.NdotH = NdotH;
    pbrInputs.LdotH = LdotH;
    pbrInputs.VdotH = VdotH;

    // Calculate the shading terms for the microfacet specular shading model
    vec3 F = specularReflection(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);

    // Calculation of analytical lighting contribution
    vec3 diffuseContrib = (1.0 - F) * diffuseBurley(pbrInputs);
    vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
    // Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
    color = NdotL * lightColor * (diffuseContrib + specContrib);
  }
  return color;
}



void main()
{
    InputAttributes tc;
    tc.uv[0] = TexCoord0;
    tc.uv[1] = TexCoord1;

    MetallicRoughnessDataGPU mat = GetMaterial();

    vec4 Kao = sampleAO(tc, mat);
    vec4 Ke  = sampleEmissive(tc, mat);
    vec4 Kd  = sampleAlbedo(tc, mat) * Color0;
    vec4 mrSample = sampleMetallicRoughness(tc, mat);

    // world-space normal
    vec3 n = normalize(Normal0);

    vec3 normalSample = sampleNormal(tc, mat).xyz;

    // normal mapping
    n = perturbNormal(n, WorldPos0, normalSample, getNormalUV(tc, mat));

    if (!gl_FrontFacing) n *= -1.0f;

    PBRInfo pbrInputs = calculatePBRInputsMetallicRoughness(Kd, n, gCameraWorldPos, WorldPos0, mrSample);

    vec3 specular_color = getIBLRadianceContributionGGX(pbrInputs, 1.0);
    vec3 diffuse_color = getIBLRadianceLambertian(pbrInputs.NdotV, n, pbrInputs.perceptualRoughness, 
                                                  pbrInputs.diffuseColor, pbrInputs.reflectance0, 1.0);
    vec3 color = specular_color + diffuse_color;

    // one hardcoded light source
    vec3 lightPos = vec3(0, 0, -5);

    vec3 LightContribution = calculatePBRLightContribution( pbrInputs, normalize(lightPos - WorldPos0), vec3(1.0) );

    color += LightContribution;

    // ambient occlusion
    color = color * ( Kao.r < 0.01 ? 1.0 : Kao.r );
    // emissive
    color = Ke.rgb + color;
    // convert to sRGB
    color = pow(color, vec3(1.0/2.2) );

    out_FragColor = vec4(color, 1.0);

// Uncomment to debug:
//  out_FragColor = vec4((n + vec3(1.0))*0.5, 1.0);
 // out_FragColor = Kao;
  //out_FragColor = Ke;
  //out_FragColor = Kd;
 // out_FragColor = mrSample;
  //vec2 MeR = mrSample.yz;
  //out_FragColor = vec4(diffuse_color, 1.0);
  //out_FragColor = vec4(specular_color, 1.0);
 // out_FragColor = vec4(LightContribution, 1.0);
//  MeR.x *= getMetallicFactor(mat);
//  MeR.y *= getRoughnessFactor(mat);
  //out_FragColor = vec4(MeR.y,MeR.y,MeR.y, 1.0);
//  out_FragColor = mrSample;
}
