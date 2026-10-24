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


layout(location = 0) out vec4 OutColor;

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


bool gCellShadingEnabled = false;
bool gEnableSpecularExponent = false;
vec3 GeoNormal;

// Light level of a fully shadowed pixel (ambient is added separately).
const float kShadowFloor = 0.05;

// ---------------------------------------------------------------------------
// Material
// ---------------------------------------------------------------------------


vec4 TextureBindless2D(uint MaterialIndex, vec2 uv) 
{
    uint Index = push.BaseTextureIndex + MaterialIndex;
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

    float bias = max(0.05 * (1.0 - DiffuseFactor), 0.0005);

    return (ShadowTest(Index, ShadowCoords.xy, ShadowCoords.z, bias) > 0.5) ? 1.0 : kShadowFloor;
}

// LightToPixel is the (unnormalized) vector from the light to the pixel.
// Point lights use the cube map; spot and directional lights use the 2D map.
float CalcShadowFactor(int Index, vec3 LightToPixel, bool IsPoint)
{
  //  if (!gShadowsEnabled) {
   //     return 1.0;
  //  }

  //  if (IsPoint) {
  //      return CalcShadowFactorPointLight(LightToPixel);
  //  }

    vec3 LightDirection = normalize(LightToPixel);

 //   if (gShadowMapRandomRadius > 0.0) {
  //      return CalcShadowFactorWithRandomSampling(LightDirection);
 //   } else if (gShadowMapFilterSize > 0) {
 //      return CalcShadowFactorPCF(LightDirection);
 //   } else {
        return CalcShadowFactorBasic(Index, LightDirection);
  //  }
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

    float DiffuseFactor = dot(GeoNormal, -LightDirection);

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

  //  if (gHasNormalMap) {
        // Fade out light on faces that point away from the light according to the
        // geometric normal. Clamped: a negative value used to subtract light.
   //     float OriginalDiffuse = max(dot(GeoNormal, -LightDirection), 0.0);
  //      DiffuseColor *= OriginalDiffuse;
   // }

    return (AmbientColor + ShadowFactor * (DiffuseColor + SpecularColor + RimColor));
   //return AmbientColor;
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
    //} else if (gIsIndirectRender) {
    //    TexColor = textureGrad(materials[MaterialIndex].DiffuseMap, TexCoord, TexDx, TexDy);
    //} else if (gHasSampler) {
    //    TexColor = textureGrad(gSampler, TexCoord, TexDx, TexDy);
    //} else {
   //     TexColor = vec4(1.0, 0.0, 0.0, 1.0);
    //}

    TexColor = TextureBindless2D(MaterialIndex, TexCoord);

    return TexColor;
}


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
    GeoNormal = normalize(Normal);

    vec4 TexColor = GetTexColor();

    vec4 BaseColor = TexColor * Color;

    vec3 LightColor = ubo.LightSources[0].Color;
    float AmbientIntensity = ubo.LightSources[0].AmbientIntensity;
    float DiffuseIntensity = ubo.LightSources[0].DiffuseIntensity;

    switch (LIGHTING_MODE) {
        case LIGHTING_MODE_UNLIT:
            OutColor = BaseColor;
            break;

        case LIGHTING_MODE_NORMALS:
            OutColor = vec4(GeoNormal * 0.5 + 0.5, 1.0);
            break;

        case LIGHTING_MODE_AMBIENT_ONLY:
            OutColor = vec4(BaseColor.rgb * LightColor * AmbientIntensity, BaseColor.a);
            break;

        case LIGHTING_MODE_FULL:
            vec4 Color = CalcPhongLighting(BaseColor);

            OutColor = Color;

            break;

        default:
            OutColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
}
