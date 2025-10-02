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

#define MAX_NUM_LIGHTS 4

#define LIGHT_TYPE_DIR   0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT  2


struct LightSource {
    vec3 Color;                    // offset 0
    int LightType;                 // offset 12
    vec3 Direction;                // offset 16
    float AmbientIntensity;        // offset 28
    vec3 WorldPos;                 // offset 32
    float DiffuseIntensity;        // offset 44
    float Atten_Constant;          // offset 48
    float Atten_Linear;            // offset 52
    float Atten_Exp;               // offset 56
    float Cutoff;                  // offset 60
};


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

    // Normal
    vec3 n;              // Sahding normal
    vec3 ng;            // Geometry normal
    vec3 t;              // Geometry tangent
    vec3 b;              // Geometry bitangent

    vec3 v;                       // vector from surface point to camera

    // material properties
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
    vec3 reflectance0;            // full reflectance color (normal incidence angle)
    vec3 reflectance90;           // reflectance color at grazing angle
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 baseDiffuseColor;            // color contribution from diffuse lighting
    vec3 baseSpecularColor;           // color contribution from specular lighting

    vec3 clearCoatF0;
    vec3 clearCoatF90;
    float clearCoatFactor;
    vec3 clearCoatNormal;
    float clearCoatRoughness;

    float ior;  // TODO

    vec3 FssEss;
    float brdf_scale;
    float brdf_bias;
};


layout(std140, binding = 1) uniform LightUBO {
    LightSource Lights[MAX_NUM_LIGHTS];
};


uniform int gNumLights = 0;
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
layout(binding = 15) uniform sampler2D gClearCoat;
layout(binding = 16) uniform sampler2D gClearCoatRoughness;
layout(binding = 17) uniform sampler2D gClearCoatNormal;
uniform bool gHasNormalMap = false;
uniform bool gHasHeightMap = false;
uniform int gShadowMapWidth = 0;
uniform int gShadowMapHeight = 0;
uniform int gShadowMapFilterSize = 0;
uniform float gShadowMapOffsetTextureSize;
uniform float gShadowMapOffsetFilterSize;
uniform float gShadowMapRandomRadius = 0.0;
uniform vec3 gCameraWorldPos;
uniform bool gShadowsEnabled = true;
uniform bool gIsIndirectRender = false;
uniform vec4 gBaseColor;
uniform vec4 gEmissiveColor;
uniform vec4 gMetallicRoughnessNormalOcclusion;
uniform vec4 gClearCoatTransmissionThickness;
uniform int gMaterialType;
uniform float gIOR;

const float M_PI = 3.141592653589793;

vec4 SRGBtoLINEAR(vec4 srgbIn) 
{
    vec3 linOut = pow(srgbIn.xyz,vec3(2.2));

    return vec4(linOut, srgbIn.a);
}


float ClampedDot(vec3 x, vec3 y) 
{
    return clamp(dot(x, y), 0.0, 1.0);
}


vec3 F_Schlick(vec3 f0, vec3 f90, float VdotH) 
{
    return f0 + (f90 - f0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}



// http://www.thetenthplanet.de/archives/1180
// modified to fix handedness of the resulting cotangent frame
// http://www.thetenthplanet.de/archives/1180
// modified to fix handedness of the resulting cotangent frame
mat3 cotangentFrame( vec3 N, vec3 p, vec2 uv, inout PBRInfo pbrInputs ) 
{
  // get edge vectors of the pixel triangle
  vec3 dp1 = dFdx( p );
  vec3 dp2 = dFdy( p );
  vec2 duv1 = dFdx( uv );
  vec2 duv2 = dFdy( uv );

  if (length(duv1) <= 1e-2) {
    duv1 = vec2(1.0, 0.0);
  }

  if (length(duv2) <= 1e-2) {
    duv2 = vec2(0.0, 1.0);
  }

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

  if (gl_FrontFacing == false) {
    N *= -1.0f;
    T *= -1.0f;
    B *= -1.0f;
  }

  pbrInputs.t = T * invmax;
  pbrInputs.b = B * invmax;
  pbrInputs.ng = N;

  return mat3( pbrInputs.t, pbrInputs.b, N );
}


void perturbNormal(vec3 n, vec3 v, vec3 normalSample, vec2 uv, inout PBRInfo pbrInputs) 
{
    vec3 map = normalize( 2.0 * normalSample - vec3(1.0) );
    mat3 TBN = cotangentFrame(n, v, uv, pbrInputs);
    pbrInputs.n = normalize(TBN * map);
}


struct InputAttributes {
    vec2 uv[2];
};


struct ClearCoat {
    sampler2D TextureSampler;
    uint TextureUV;
    sampler2D RoughnessSampler;
    uint RoughnessUV;
    sampler2D NormalSampler;
    uint NormalUV;
    vec4 TransmissionThickness;
};


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
    ClearCoat clearCoat;
    uint materialType;  // TODO
    float ior; // TODO
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
        ret.materialType = gMaterialType;
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
        ret.metallicRoughnessNormalOcclusion = gMetallicRoughnessNormalOcclusion;
        ret.clearCoat.TextureSampler = gClearCoat;
        ret.clearCoat.TextureUV = 0;
        ret.clearCoat.RoughnessSampler = gClearCoatRoughness;
        ret.clearCoat.RoughnessUV = 0;
        ret.clearCoat.NormalSampler = gClearCoatNormal;
        ret.clearCoat.NormalUV = 0;
        ret.clearCoat.TransmissionThickness = gClearCoatTransmissionThickness;
        ret.ior = gIOR;
        return ret;
    }      
}

EnvironmentMapDataGPU getEnvironment(uint idx) 
{
    if (gIsIndirectRender) {
        return environment[idx]; 
    } else {
        EnvironmentMapDataGPU ret;
        ret.envMapTextureSampler = gEnvMap;
        ret.envMapTextureIrradianceSampler = gIrradiance;
        ret.texBRDF_LUTSampler = gBRDF_LUT;
        return ret;
    }  
}

float GetMetallicFactor(MetallicRoughnessDataGPU mat) 
{
    return mat.metallicRoughnessNormalOcclusion.x;
}

float GetRoughnessFactor(MetallicRoughnessDataGPU mat) 
{
    return mat.metallicRoughnessNormalOcclusion.y;
}

float GetNormalScale(MetallicRoughnessDataGPU mat) 
{
    return mat.metallicRoughnessNormalOcclusion.z;
}

float GetOcclusionFactor(MetallicRoughnessDataGPU mat) 
{
    return mat.metallicRoughnessNormalOcclusion.w;
}

vec2 GetNormalUV(InputAttributes tc, MetallicRoughnessDataGPU mat) 
{
    return tc.uv[mat.normalTextureUV];
}


float GetClearcoatFactor(InputAttributes tc, MetallicRoughnessDataGPU mat) 
{
    return texture(mat.clearCoat.TextureSampler, 
                   tc.uv[mat.clearCoat.TextureUV]).r * mat.clearCoat.TransmissionThickness.x;
}


float GetClearcoatRoughnessFactor(InputAttributes tc, MetallicRoughnessDataGPU mat) 
{
    return texture(mat.clearCoat.RoughnessSampler, 
                   tc.uv[mat.clearCoat.RoughnessUV]).g * mat.clearCoat.TransmissionThickness.y;
}


vec4 sampleAO(InputAttributes tc, MetallicRoughnessDataGPU mat) {
    return texture(mat.occlusionTextureSampler, tc.uv[mat.occlusionTextureUV]);
}

vec4 SampleEmissive(InputAttributes tc, MetallicRoughnessDataGPU mat) 
{
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


vec4 SampleClearcoatNormal(InputAttributes tc, MetallicRoughnessDataGPU mat) 
{
    return texture(mat.clearCoat.NormalSampler, tc.uv[mat.clearCoat.NormalUV]);
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
    return textureLod(map.envMapTextureSampler, tc, lod);
}


bool isMaterialTypeClearCoat(MetallicRoughnessDataGPU mat) 
{
    return (mat.materialType & 0x8) != 0;
}


// Based on: https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/pbr.frag


vec3 getIBLRadianceLambertian(PBRInfo pbrInputs) 
{
    EnvironmentMapDataGPU envMap = getEnvironment(getEnvironmentId());

    vec3 Irradiance = texture(envMap.envMapTextureIrradianceSampler, pbrInputs.n).rgb;

    // Multiple scattering, from Fdez-Aguera
    float Ems = (1.0 - (pbrInputs.brdf_scale + pbrInputs.brdf_bias));
    vec3 F_avg = (pbrInputs.reflectance0 + (1.0 - pbrInputs.reflectance0) / 21.0);
    vec3 FmsEms = F_avg * Ems * pbrInputs.FssEss / (1.0 - F_avg * Ems);
    // we use +FmsEms as indicated by the formula in the blog post 
    // (might be a typo in the implementation)
    vec3 k_D = pbrInputs.baseDiffuseColor * (1.0 - pbrInputs.FssEss + FmsEms); 

    return (FmsEms + k_D) * Irradiance;
}

// Calculation of the lighting contribution from an optional Image Based Light source.
// Precomputed Environment Maps are required uniform inputs and are computed as outlined in [1].
// See our README.md on Environment Maps [3] for additional discussion.
vec3 getIBLRadianceGGX(vec3 n, vec3 v, float roughness, vec3 F0) 
{
    float NdotV = ClampedDot(n, v);
    EnvironmentMapDataGPU envMap = getEnvironment(getEnvironmentId());       
    float mipCount = float(textureQueryLevels(envMap.envMapTextureSampler));
    float lod = roughness * (mipCount - 1);
    vec3 reflection = normalize(reflect(-v, n));
    
    // HDR envmaps are already linear
    vec3 specularLight = sampleEnvMapLod(reflection.xyz, lod, envMap).rgb;

    vec2 brdfSamplePoint = clamp(vec2(NdotV, roughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
    vec3 f_ab = sampleBRDF_LUT(brdfSamplePoint, envMap).rgb;

    // see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
    // Roughness dependent fresnel, from Fdez-Aguera
    vec3 Fr = max(vec3(1.0 - roughness), F0) - F0;
    vec3 k_S = F0 + Fr * pow(1.0 - NdotV, 5.0);
    vec3 FssEss = k_S * f_ab.x + f_ab.y;

    return specularLight * FssEss;
}

// Disney Implementation of diffuse from Physically-Based Shading at Disney by Brent Burley. See Section 5.3.
// http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
vec3 diffuseBurley(PBRInfo pbrInputs) 
{
    float f90 = 2.0 * pbrInputs.LdotH * pbrInputs.LdotH * pbrInputs.alphaRoughness - 0.5;

    return (pbrInputs.baseDiffuseColor / M_PI) * (1.0 + f90 * pow((1.0 - pbrInputs.NdotL), 5.0)) * 
                                                 (1.0 + f90 * pow((1.0 - pbrInputs.NdotV), 5.0));
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs) 
{
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * 
           pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs) 
{
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
float microfacetDistribution(PBRInfo pbrInputs) 
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (M_PI * f * f);
}


vec3 CalcSchlickFresnel(inout PBRInfo pbrInputs)
{
    EnvironmentMapDataGPU envMap = getEnvironment(getEnvironmentId());

    vec2 brdfSamplePoint = vec2(pbrInputs.NdotV, pbrInputs.perceptualRoughness);
    brdfSamplePoint = clamp(brdfSamplePoint, vec2(0.0, 0.0), vec2(1.0, 1.0));
    vec2 brdf = sampleBRDF_LUT(brdfSamplePoint, envMap).rg;
    pbrInputs.brdf_scale = brdf.x;
    pbrInputs.brdf_bias = brdf.y;

    // see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
    // Roughness dependent fresnel, from Carmelo Fdez-Aguera
    vec3 Fr = max(vec3(1.0 - pbrInputs.perceptualRoughness), pbrInputs.reflectance0) - pbrInputs.reflectance0;
    vec3 k_S = pbrInputs.reflectance0 + Fr * pow(1.0 - pbrInputs.NdotV, 5.0);
    vec3 FssEss = k_S * pbrInputs.brdf_scale + pbrInputs.brdf_bias;

    return FssEss;
}


PBRInfo CalculatePBRInputsMetallicRoughness(MetallicRoughnessDataGPU mat, 
                                            vec4 albedo, vec3 normal, vec4 mrSample) 
{
    PBRInfo pbrInputs;
    pbrInputs.ior = mat.ior;
  
    // Roughness is stored in the 'g' channel, MetallicFactor is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    
    float MetallicFactor = GetMetallicFactor(mat) * mrSample.b;
    MetallicFactor = clamp(MetallicFactor, 0.0, 1.0);

    float PerceptualRoughness = GetRoughnessFactor(mat);
    PerceptualRoughness = mrSample.g * PerceptualRoughness;
    const float c_MinRoughness = 0.04;
    PerceptualRoughness = clamp(PerceptualRoughness, c_MinRoughness, 1.0);

    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness [2].
    float alphaRoughness = PerceptualRoughness * PerceptualRoughness;

    vec3 f0 = vec3(0.04);
    vec3 baseDiffuseColor = mix(albedo.rgb, vec3(0), MetallicFactor); 
    vec3 baseSpecularColor = mix(f0, albedo.rgb, MetallicFactor);

    float reflectance = max(max(baseSpecularColor.r, baseSpecularColor.g), baseSpecularColor.b);

    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec3 specularEnvironmentR0 = baseSpecularColor.rgb;
    vec3 specularEnvironmentR90 = vec3(reflectance90);

    vec3 v = normalize(gCameraWorldPos - WorldPos0);

    pbrInputs.NdotV = clamp(abs(dot(normal, v)), 0.001, 1.0);
    pbrInputs.perceptualRoughness = PerceptualRoughness;
    pbrInputs.reflectance0 = specularEnvironmentR0;
    pbrInputs.reflectance90 = specularEnvironmentR90;
    pbrInputs.alphaRoughness = alphaRoughness;
    pbrInputs.baseDiffuseColor = baseDiffuseColor;
    pbrInputs.baseSpecularColor = baseSpecularColor;
    pbrInputs.n = normal;
    pbrInputs.v = v;

    pbrInputs.FssEss = CalcSchlickFresnel(pbrInputs);

    return pbrInputs;
}


vec3 calculatePBRLightContribution(inout PBRInfo pbrInputs, vec3 LightDirection, vec3 lightColor) 
{
    vec3 n = pbrInputs.n;
    vec3 v = pbrInputs.v;
    vec3 l = normalize(LightDirection);  // Vector from surface point to light
    vec3 h = normalize(l+v);             // Half vector between both l and v

    float NdotV = pbrInputs.NdotV;
    float NdotL = clamp(dot(n, l), 0.001, 1.0);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
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
    EnvironmentMapDataGPU envMap = getEnvironment(getEnvironmentId());
          
    vec4 AmbientOcclusion = sampleAO(tc, mat);
    vec4 EmissiveColor = SampleEmissive(tc, mat);
    vec4 AlbedoColor = sampleAlbedo(tc, mat) * Color0;
    vec4 mrSample = sampleMetallicRoughness(tc, mat);
    vec3 normalSample = sampleNormal(tc, mat).xyz;

    bool isClearCoat = isMaterialTypeClearCoat(mat);

    vec3 n = normalize(Normal0);

    PBRInfo pbrInputs = CalculatePBRInputsMetallicRoughness(mat, AlbedoColor, n, mrSample);

    perturbNormal(n, WorldPos0, normalSample, GetNormalUV(tc, mat), pbrInputs);

   // if (!gl_FrontFacing) n *= -1.0f;    
    
    vec3 ClearCoatContrib = vec3(0);

    if (isClearCoat) {
      pbrInputs.clearCoatFactor = GetClearcoatFactor(tc, mat);
      pbrInputs.clearCoatRoughness = clamp(GetClearcoatRoughnessFactor(tc, mat), 0.0, 1.0);
      pbrInputs.clearCoatF0 = vec3(pow((pbrInputs.ior - 1.0) / (pbrInputs.ior + 1.0), 2.0));
      pbrInputs.clearCoatF90 = vec3(1.0);

      if (mat.clearCoat.NormalUV >- 1) {
          pbrInputs.clearCoatNormal = mat3(pbrInputs.t, pbrInputs.b, pbrInputs.ng) * SampleClearcoatNormal(tc, mat).rgb;
      } else {
          pbrInputs.clearCoatNormal = pbrInputs.ng;
      }

      ClearCoatContrib = getIBLRadianceGGX(pbrInputs.clearCoatNormal, pbrInputs.v, 
                                           pbrInputs.clearCoatRoughness, pbrInputs.clearCoatF0);
    }

    vec3 ClearCoatFresnel = vec3(0);
    if (isClearCoat) {
        ClearCoatFresnel = F_Schlick(pbrInputs.clearCoatF0, pbrInputs.clearCoatF90, 
                                     ClampedDot(pbrInputs.clearCoatNormal, pbrInputs.v));
    }

    float Occlusion = AmbientOcclusion.r < 0.01 ? 1.0 : AmbientOcclusion.r;
    float OcclusionStrength = GetOcclusionFactor(mat);
    vec3 SpecularColor = getIBLRadianceGGX(pbrInputs.n, pbrInputs.v, 
                                           pbrInputs.perceptualRoughness, pbrInputs.reflectance0);
    vec3 DiffuseColor = getIBLRadianceLambertian(pbrInputs);
    DiffuseColor = /*lights_diffuse + */ mix(DiffuseColor, DiffuseColor * Occlusion, OcclusionStrength);
    SpecularColor = /*lights_specular +*/ mix(SpecularColor, SpecularColor * Occlusion, OcclusionStrength);

    vec3 LightDirection = vec3(1.0, -1.0, 0.0); // default light if none defined
    vec3 LightColor = vec3(1.0);

    if (gNumLights > 0) {
        LightDirection = normalize(Lights[0].WorldPos - WorldPos0);
        LightColor = Lights[0].Color;
    }

    vec3 LightContribution = calculatePBRLightContribution(pbrInputs, LightDirection, LightColor);

    vec3 Color = SpecularColor + DiffuseColor + LightContribution + EmissiveColor.rgb;
    Color = Color * (1.0 - pbrInputs.clearCoatFactor * ClearCoatFresnel) + ClearCoatContrib;

    // convert to sRGB
    Color = pow(Color, vec3(1.0/2.2) );

    out_FragColor = vec4(Color, 1.0);

// Uncomment to debug:
//  out_FragColor = vec4((n + vec3(1.0))*0.5, 1.0);
 // out_FragColor = AmbientOcclusion;
  //out_FragColor = EmissiveColor;
  //out_FragColor = AlbedoColor;
 // out_FragColor = mrSample;
  //vec2 MeR = mrSample.yz;
 // out_FragColor = vec4(DiffuseColor, 1.0);
 // out_FragColor = vec4(SpecularColor, 1.0);
 // out_FragColor = vec4(LightContribution, 1.0);
//  MeR.x *= GetMetallicFactor(mat);
//  MeR.y *= GetRoughnessFactor(mat);
  //out_FragColor = vec4(MeR.y,MeR.y,MeR.y, 1.0);
//  out_FragColor = mrSample;

  //  float MetallicFactor = GetMetallicFactor(mat) * mrSample.b;
  //  MetallicFactor = clamp(MetallicFactor, 0.0, 1.0);
  //  out_FragColor = vec4(MetallicFactor);

  //    float PerceptualRoughness = GetRoughnessFactor(mat);
  //  PerceptualRoughness = mrSample.g * PerceptualRoughness;
  //  const float c_MinRoughness = 0.04;
  //  PerceptualRoughness = clamp(PerceptualRoughness, c_MinRoughness, 1.0);
  //  out_FragColor = vec4(PerceptualRoughness);

  //out_FragColor = vec4(pbrInputs.FssEss, 1.0);
}
