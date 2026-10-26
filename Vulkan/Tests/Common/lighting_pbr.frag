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


#version 460

#extension GL_EXT_nonuniform_qualifier : require

#define LIGHTING_MODE_UNLIT 0
#define LIGHTING_MODE_NORMALS 1
#define LIGHTING_MODE_AMBIENT_ONLY 2
#define LIGHTING_MODE_FULL 3

#define MAX_NUM_LIGHTS 16

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
    vec4 Config;				   // offset 64 x = Enabled, y = Casts shadows, z = bias, w = filter size
};


layout(push_constant) uniform Constants {
    uint BaseTextureIndex; 
} push;

layout(constant_id = 0) const uint LIGHTING_MODE = LIGHTING_MODE_FULL;

layout(location = 0) in vec2 TexCoord;
layout(location = 1) flat in uint MaterialIndex;
layout(location = 2) in vec3 WorldPos;
layout(location = 3) in vec3 Normal;
layout(location = 4) in vec4 Color;
layout(location = 5) in vec3 Tangent;
layout(location = 6) in vec3 Bitangent;

layout(location = 0) out vec4 OutColor;

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
    vec3 n;              // Shading normal
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
    float MetallicFactor;              // metallic factor, as authored by the model creator (input to shader)

    vec3 clearCoatF0;
    vec3 clearCoatF90;
    float clearCoatFactor;
    vec3 clearCoatNormal;

    float ior;

    vec3 FssEss;
    float brdf_scale;
    float brdf_bias;
};


struct InputAttributes {
    vec2 uv[2];
};


layout(set = 0, binding = 0) uniform sampler2D Textures[];

layout(set = 1, binding = 4) readonly uniform UniformBuffer { 
    ivec4 Counts;              // .x = NumLights, .y = NumPointLights, .z = NumSpotLights, .w = NumDirectionalLights
    vec4 CameraWorldPos;
    vec4 Material;            // .x = specular intensity
    LightSource LightSources[MAX_NUM_LIGHTS];
} ubo;

layout(set = 1, binding = 5) uniform sampler2D ShadowMaps[MAX_NUM_LIGHTS];

layout(set = 1, binding = 6) uniform LightMats {
    ivec4 Counts;              // .x = NumLights, .y = NumPointLights, .z = NumSpotLights, .w = NumDirectionalLights
    mat4 LightVP[MAX_NUM_LIGHTS];
} LightMatrices;

struct MaterialData { 
    ivec4 Indices; // .x = BaseColorIndex, .y = NormalMapIndex, .z = metallic roughness, .w = AO
    ivec4 Indices2; // .x = Emissive, .y = clear coat, .z = clear coat roughness, .w = clear coat thickness
    vec4 m_baseColorFactor;
    vec4 m_metallicRoughnessNormalOcclusion;
    vec4 m_emmisiveFactorAlphaCutoff;
    vec4 m_clearCoatTransmissionThinkness;
    vec4 m_typeIORAlpha; // x. material type, y. IOR, z. alpha mode, w. unused
};

layout(std430, set = 1, binding = 7) readonly buffer MaterialSSBO { 
    MaterialData materials[]; 
} MaterialBuf;

bool gCellShadingEnabled = false;
bool gEnableSpecularExponent = false;
vec3 GeoNormal;
float gShadowMapWidth = 2048.0;
float gShadowMapHeight = 2048.0;

// Light level of a fully shadowed pixel (ambient is added separately).
const float kShadowFloor = 0.05;

// ---------------------------------------------------------------------------
// Material
// ---------------------------------------------------------------------------


vec4 TextureBindless2D(uint TextureIndex, vec2 uv) 
{
    uint Index = push.BaseTextureIndex + TextureIndex;
    return texture(Textures[nonuniformEXT(Index)], uv);
}


vec4 GetMaterialAmbientColor()  
{ 
    return vec4(1.0); 
}


vec4 GetMaterialDiffuseColor()  
{ 
    return vec4(1.0); 
}


vec4 GetMaterialSpecularColor() 
{ 
    return vec4(ubo.Material.x); 
}


// ---------------------------------------------------------------------------
// Shadows
// ---------------------------------------------------------------------------

bool IsOutsideShadowMap(vec3 ShadowCoords)
{
    return ShadowCoords.z > 1.0 ||
           ShadowCoords.z < 0 ||
           any(lessThan(ShadowCoords.xy, vec2(0.0))) ||
           any(greaterThan(ShadowCoords.xy, vec2(1.0)));
}


// Returns 1.0 if lit, 0.0 if shadowed. Always samples mip 0 (shadow maps are not mip-filtered)
// which also makes the call safe inside divergent branches.
float ShadowTest(int Index, vec2 UV, float RefZ, float Bias)
{
    float Depth = textureLod(ShadowMaps[Index], UV, 0.0).x;
    return (Depth + Bias < RefZ) ? 0.0 : 1.0;
}


vec3 CalcShadowCoords(int Index)
{
    vec4 LightSpacePos = LightMatrices.LightVP[Index] * vec4(WorldPos, 1.0);
    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;
    vec2 XY = vec2(ProjCoords.x * 0.5 + 0.5, -ProjCoords.y * 0.5 + 0.5);
    vec3 ShadowCoords = vec3(XY, ProjCoords.z);
    return ShadowCoords;
}


float CalcShadowFactorBasic(int Index, vec3 LightDirection)
{
    vec3 ShadowCoords = CalcShadowCoords(Index);

    if (IsOutsideShadowMap(ShadowCoords)) {
        return 1.0;
    }

    float DiffuseFactor = clamp(dot(GeoNormal, -LightDirection), 0.0, 1.0);

    float bias;
    
    float ConfiguredBias = ubo.LightSources[Index].Config.z;
    if (ConfiguredBias > 0.0) {
        bias = ConfiguredBias;
    } else {
        bias = max(0.05 * (1.0 - DiffuseFactor), 0.0005);
    }

    float ShadowFactor = (ShadowTest(Index, ShadowCoords.xy, ShadowCoords.z, bias) > 0.5) ? 1.0 : kShadowFloor;
    
    return ShadowFactor;
}


float CalcShadowFactorPCF(int Index, vec3 LightDirection)
{
    if (gShadowMapWidth == 0 || gShadowMapHeight == 0) {
        return 1.0;
    }

    vec3 ShadowCoords = CalcShadowCoords(Index);

    if (IsOutsideShadowMap(ShadowCoords)) {
        return 1.0;
    }

    float DiffuseFactor = clamp(dot(GeoNormal, -LightDirection), 0.0, 1.0);
    float bias = mix(0.001, 0.0, DiffuseFactor);

    vec2 TexelSize = 1.0 / vec2(gShadowMapWidth, gShadowMapHeight);

    float ShadowSum = 0.0;

    int ShadowMapFilterSize = int(ubo.LightSources[Index].Config.w);

    int HalfFilterSize = ShadowMapFilterSize / 2;

    for (int y = -HalfFilterSize ; y < -HalfFilterSize + ShadowMapFilterSize ; y++) {
        for (int x = -HalfFilterSize ; x < -HalfFilterSize + ShadowMapFilterSize ; x++) {
            vec2 Offset = vec2(x, y) * TexelSize;
            ShadowSum += ShadowTest(Index, ShadowCoords.xy + Offset, ShadowCoords.z, bias);
        }
    }

    float Lit = ShadowSum / float(ShadowMapFilterSize * ShadowMapFilterSize);

    return mix(kShadowFloor, 1.0, Lit);
}


// LightToPixel is the (unnormalized) vector from the light to the pixel.
// Point lights use the cube map; spot and directional lights use the 2D map.
float CalcShadowFactor(int Index, vec3 LightToPixel, bool IsPoint)
{
    float ShadowFactor = 1.0;

    if (ubo.LightSources[Index].Config.y > 0.0) {

      //  if (IsPoint) {
      //      ShadowFactor = CalcShadowFactorPointLight(LightToPixel);
      //  }

        vec3 LightDirection = normalize(LightToPixel);

        int ShadowMapFilterSize = int(ubo.LightSources[Index].Config.w);
     //   if (gShadowMapRandomRadius > 0.0) {
      //      ShadowFactor = CalcShadowFactorWithRandomSampling(LightDirection);
        if (ShadowMapFilterSize > 0) {
           ShadowFactor = CalcShadowFactorPCF(Index, LightDirection);
        } else {
            ShadowFactor = CalcShadowFactorBasic(Index, LightDirection);
        }
    }

    return ShadowFactor;
}

// ---------------------------------------------------------------------------
// Normal mapping
// ---------------------------------------------------------------------------

vec3 CalcBumpedNormal()
{
    int NormalTextureIndex = MaterialBuf.materials[MaterialIndex].Indices.y;

    if (NormalTextureIndex < 0) {
        return GeoNormal;
    }

    // Step #1: re-normalize the tangent space base vectors after interpolation
    vec3 N = GeoNormal;
    vec3 T = normalize(Tangent - dot(Tangent, N) * N);
    vec3 B = cross(N, T) * (dot(cross(N, T), Bitangent) < 0.0 ? -1.0 : 1.0);

    // Step #2: Sample a normal from the normal map
    vec3 BumpMapNormal = TextureBindless2D(NormalTextureIndex, TexCoord).xyz;

    // Step #3: transform from [0,1] to [-1,1]
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0);

    mat3 TBN = mat3(T, B, N);

    // Step #4: transform from tangent space to world space
    vec3 NewNormal = normalize(TBN * BumpMapNormal);

    return NewNormal;                                                                       
}


// ---------------------------------------------------------------------------
// Lighting
// ---------------------------------------------------------------------------

vec4 CalcLightInternal(int Index, vec3 LightDirection, float ShadowFactor)
{
    LightDirection = normalize(LightDirection);

    vec4 AmbientColor = vec4(ubo.LightSources[Index].Color, 1.0f) *
                        ubo.LightSources[Index].AmbientIntensity *
                        GetMaterialAmbientColor();

    vec3 N = CalcBumpedNormal();

    float DiffuseFactor = dot(N, -LightDirection);

    vec4 DiffuseColor = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);
    vec4 RimColor = vec4(0, 0, 0, 0);

    if (DiffuseFactor > 0) {
     //   if (gCellShadingEnabled) {
     //       DiffuseFactor = ceil(DiffuseFactor * toon_color_levels) * toon_scale_factor;
    //    }

        DiffuseColor = vec4(ubo.LightSources[Index].Color, 1.0f) *
                       ubo.LightSources[Index].DiffuseIntensity *
                       GetMaterialDiffuseColor() *
                       DiffuseFactor;

        vec3 PixelToCamera = normalize(ubo.CameraWorldPos.xyz - WorldPos);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(PixelToCamera, LightReflect);

        if (!gCellShadingEnabled && (SpecularFactor > 0)) {
            float SpecularExponent = 128.0;

            if (gEnableSpecularExponent) {
                // Inside a divergent branch: explicit gradients keep the LOD well defined.
             //   SpecularExponent = textureGrad(gSamplerSpecularExponent, TexCoord, TexDx, TexDy).r * 255.0;
            //    SpecularExponent = max(SpecularExponent, 1.0);   // pow(x, 0) == 1 gave full-power specular
            }

            SpecularFactor = pow(SpecularFactor, SpecularExponent);
            SpecularColor = vec4(ubo.LightSources[Index].Color, 1.0f) *
                                 ubo.LightSources[Index].DiffuseIntensity * // using the diffuse intensity for diffuse/specular
                                 GetMaterialSpecularColor() *
                                 SpecularFactor;
        }

     //   if (gRimLightEnabled) {
      //     float RimFactor = CalcRimLightFactor(PixelToCamera, Normal);
      //     RimColor = DiffuseColor * RimFactor;
      //  }
    }

    // Geometric horizon: keep full light on front faces, fade quickly to 0 at the terminator.
    // Prevents bumped normals from picking up light on faces that point away from it.
    float Horizon = smoothstep(0.0, 0.1, dot(GeoNormal, -LightDirection));
    DiffuseColor  *= Horizon;
    SpecularColor *= Horizon;

    return (AmbientColor + ShadowFactor * (DiffuseColor + SpecularColor + RimColor));
}



vec4 CalcDirectionalLight(int Index)
{
    vec3 Direction = normalize(ubo.LightSources[Index].Direction);
    float ShadowFactor = CalcShadowFactor(Index, Direction, false);
    return CalcLightInternal(Index, Direction, ShadowFactor);
}


vec4 CalcPointLight(int Index, bool IsPoint)
{
    vec3 LightWorldDir = WorldPos - ubo.LightSources[Index].WorldPos;   // deliberately not normalized yet
    float ShadowFactor = CalcShadowFactor(Index, LightWorldDir, IsPoint);

    float Distance = length(LightWorldDir);
    LightWorldDir = LightWorldDir / max(Distance, 1e-6);
    vec4 Color = CalcLightInternal(Index, LightWorldDir, ShadowFactor);

    // Note: dividing by (1 + attenuation) is non-standard; with Atten_Constant = 1 the
    // light is already halved at distance 0. Kept as-is to preserve the current look.
    float Attenuation =  ubo.LightSources[Index].Atten_Constant +
                         ubo.LightSources[Index].Atten_Linear * Distance +
                         ubo.LightSources[Index].Atten_Exp * Distance * Distance;

    return Color / (1.0 + Attenuation);
}


vec4 CalcSpotLight(int Index)
{
    vec3 PixelToLight = normalize(ubo.LightSources[Index].WorldPos - WorldPos);
    float SpotFactor = dot(PixelToLight, -normalize(ubo.LightSources[Index].Direction));

    vec4 Color = vec4(0);

    if (SpotFactor > ubo.LightSources[Index].Cutoff) {
        Color = CalcPointLight(Index, false);
        float SpotLightIntensity = (1.0 - (1.0 - SpotFactor)/(1.0 - ubo.LightSources[Index].Cutoff));
        Color *= SpotLightIntensity;
    }

    return Color;
}


vec4 GetTotalLight()
{
    vec4 TotalLight = vec4(0.0);

    int NumLights = ubo.Counts.x;

    for (int i = 0 ; i < NumLights ; i++) {
        if (ubo.LightSources[i].Config.x > 0.5) {
            switch (ubo.LightSources[i].LightType) {

                case LIGHT_TYPE_DIR:
                    TotalLight += CalcDirectionalLight(i);
                    break;

                case LIGHT_TYPE_POINT:
                    TotalLight += CalcPointLight(i, true);
                    break;

                case LIGHT_TYPE_SPOT:
                    TotalLight += CalcSpotLight(i);
                    break;
            }
        }
    }

    return TotalLight;
}


// ---------------------------------------------------------------------------
// Texture / reflection / refraction
// ---------------------------------------------------------------------------

vec4 GetTexColor()
{
    vec4 TexColor;

    //if (gIsCubemapping) {
    //    vec3 CameraToPixel = normalize(WorldPos - ubo.CameraWorldPos.xyz);
    //    vec3 ReflectionDir = normalize(reflect(CameraToPixel, GeoNormal));   // reflect() needs a unit normal
    //    TexColor = vec4(textureLod(gCubemapTexture, ReflectionDir, gCubeMipmapLevel).rgb, 1.0);
    //} else {
   //     TexColor = vec4(1.0, 0.0, 0.0, 1.0);
    //}

    int BaseColorTextureIndex = MaterialBuf.materials[MaterialIndex].Indices.x;

    TexColor = TextureBindless2D(BaseColorTextureIndex, TexCoord);

    if (TexColor.a < 0.5) {
        discard;
    }

    return TexColor;
}


vec4 sampleAO(InputAttributes tc) 
{
    int AOTextureIndex = MaterialBuf.materials[MaterialIndex].Indices.w;

    if (AOTextureIndex >= 0) {
        return TextureBindless2D(AOTextureIndex, tc.uv[0]);
    } else {
        return vec4(1.0);
    }    
}


vec4 SampleEmissive(InputAttributes tc) 
{
    int EmissiveTextureIndex = MaterialBuf.materials[MaterialIndex].Indices2.x;

    vec4 EmissiveFactor = vec4(MaterialBuf.materials[MaterialIndex].m_emmisiveFactorAlphaCutoff.xyz, 1.0f);

    if (EmissiveTextureIndex >= 0) {        
        return TextureBindless2D(EmissiveTextureIndex, tc.uv[0]) * EmissiveFactor;
    } else {
        return EmissiveFactor;
    }
}


vec4 sampleAlbedo(InputAttributes tc) 
{
    int BaseTextureIndex = MaterialBuf.materials[MaterialIndex].Indices.x;
    vec4 BaseColorFactor = MaterialBuf.materials[MaterialIndex].m_baseColorFactor;
    return TextureBindless2D(BaseTextureIndex, tc.uv[0]) * BaseColorFactor;
}


vec4 sampleMetallicRoughness(InputAttributes tc) 
{
    int MRTextureIndex = MaterialBuf.materials[MaterialIndex].Indices.z;

    if (MRTextureIndex >= 0) {
        return TextureBindless2D(MRTextureIndex, tc.uv[0]);
    } else {
        return vec4(1.0);
    }
}


// Disney Implementation of diffuse from Physically-Based Shading at Disney by Brent Burley. See Section 5.3.
// http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
/*vec3 diffuseBurley(PBRInfo pbrInputs) 
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
*/


PBRInfo CalculatePBRInputsMetallicRoughness(vec4 albedo, vec3 normal, vec4 mrSample) 
{
    PBRInfo pbrInputs;
    pbrInputs.ior = MaterialBuf.materials[MaterialIndex].m_typeIORAlpha.y;
  
    // Roughness is stored in the 'g' channel, MetallicFactor is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    
    float MetallicFactor = MaterialBuf.materials[MaterialIndex].m_metallicRoughnessNormalOcclusion.x * mrSample.b;
    MetallicFactor = clamp(MetallicFactor, 0.0, 1.0);

    float PerceptualRoughness = MaterialBuf.materials[MaterialIndex].m_metallicRoughnessNormalOcclusion.y;
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

    vec3 v = normalize(ubo.CameraWorldPos.xyz - WorldPos);

    pbrInputs.NdotV = clamp(abs(dot(normal, v)), 0.001, 1.0);
    pbrInputs.perceptualRoughness = PerceptualRoughness;
    pbrInputs.reflectance0 = specularEnvironmentR0;
    pbrInputs.reflectance90 = specularEnvironmentR90;
    pbrInputs.alphaRoughness = alphaRoughness;
    pbrInputs.baseDiffuseColor = baseDiffuseColor;
    pbrInputs.baseSpecularColor = baseSpecularColor;
    pbrInputs.n = normal;
    pbrInputs.v = v;
    pbrInputs.MetallicFactor = MetallicFactor;
    pbrInputs.clearCoatFactor = 0.0;

   // pbrInputs.FssEss = CalcSchlickFresnel(pbrInputs);

    return pbrInputs;
}

/*
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

*/

vec4 CalcPhongLighting(vec4 BaseColor)
{
    vec4 TotalLight = GetTotalLight();

    vec4 TexColor = BaseColor;

    // gColorMod and gColorAdd are used to enhance the color in youtube thumbnails.
    // They are not an integral part of the lighting equation.
    //TexColor = TexColor * gColorMod + gColorAdd;

    vec4 FinalColor = TexColor * TotalLight;

    //if (gRefRefractEnabled) {
    //    FinalColor = ApplyRefRefract(FinalColor, Normal);
    //}

    // The light alphas add up (ambient uses 1.0 per light), so take alpha from the texture.
    FinalColor.a = TexColor.a;

    // HDR tone mapping / gamma correction: do these as the very last step, ideally via an
    // sRGB framebuffer (GL_FRAMEBUFFER_SRGB) and sRGB textures instead of by hand.
    // FinalColor.rgb = FinalColor.rgb / (FinalColor.rgb + vec3(1.0));
    // FinalColor.rgb = pow(FinalColor.rgb, vec3(1.0/2.2));

    return FinalColor;
}


// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

void main() 
{
    InputAttributes tc;
    tc.uv[0] = TexCoord;
    //tc.uv[1] = TexCoord1;

    GeoNormal = normalize(Normal);
   
    vec4 AmbientOcclusion = sampleAO(tc);
    vec4 EmissiveColor = SampleEmissive(tc);
    vec4 AlbedoColor = sampleAlbedo(tc);
    vec4 mrSample = sampleMetallicRoughness(tc);

    vec3 N = CalcBumpedNormal();

    vec4 TexColor = GetTexColor();
    vec4 BaseColor = TexColor * Color;    
    
    PBRInfo pbrInputs = CalculatePBRInputsMetallicRoughness(AlbedoColor, N, mrSample);

    switch (LIGHTING_MODE) {
        case LIGHTING_MODE_UNLIT:
            OutColor = BaseColor;
            break;

        case LIGHTING_MODE_NORMALS:
            OutColor = vec4(GeoNormal * 0.5 + 0.5, 1.0);
            break;

        case LIGHTING_MODE_AMBIENT_ONLY:
        //    OutColor = vec4(BaseColor.rgb * LightColor * AmbientIntensity, BaseColor.a);
            break;

        case LIGHTING_MODE_FULL:
            vec4 Color = CalcPhongLighting(BaseColor);

            OutColor = Color;

            break;

        default:
            OutColor = vec4(1.0, 0.0, 0.0, 1.0);
    }

    //OutColor = vec4(normalize(Bitangent) * 0.5 + 0.5, 1);

    //int BaseColorTextureIndex = MaterialBuf.materials[MaterialIndex].Indices.x;
    //OutColor = TextureBindless2D(BaseColorTextureIndex, TexCoord) * Color * MaterialBuf.materials[MaterialIndex].m_baseColorFactor;

    //int MetallicTextureIndex = MaterialBuf.materials[MaterialIndex].Indices.z;
    //OutColor = vec4(TextureBindless2D(MetallicTextureIndex, TexCoord).y);

    //int AOTextureIndex = MaterialBuf.materials[MaterialIndex].Indices.w;
    //OutColor = TextureBindless2D(AOTextureIndex, TexCoord);

//    int EmmisiveTextureIndex = MaterialBuf.materials[MaterialIndex].Indices2.x;
  //  OutColor = TextureBindless2D(EmmisiveTextureIndex, TexCoord);

    //OutColor = vec4(TexColor.rgb, 1.0);

    //OutColor = vec4(CalcBumpedNormal(), 1.0);

    //OutColor = vec4(vec3(pbrInputs.perceptualRoughness), 1.0);   // gradient on MetalRoughSpheres
  //  OutColor = vec4(vec3(pbrInputs.MetallicFactor), 1.0);                  // (expose it via pbrInputs or recompute)
  //  OutColor = vec4(pbrInputs.reflectance0, 1.0);                // 0.04 grey for non-metals, albedo for metals
   // OutColor = vec4(pbrInputs.n * 0.5 + 0.5, 1.0);               // must show the bump detail

}
