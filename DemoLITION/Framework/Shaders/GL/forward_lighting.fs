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

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 WorldPos0;
in vec4 LightSpacePos0;
in vec3 Tangent0;
in vec3 Bitangent0;
in flat int MaterialIndex;

out vec4 FragColor;

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


struct MaterialColor
{
    vec4 AmbientColor;
    vec4 DiffuseColor;
    vec4 SpecularColor;
};


struct Material {
    MaterialColor Color;
    sampler2D DiffuseMap;
    sampler2D NormalMap;
};

layout(std140, binding = 1) uniform LightUBO {
    LightSource Lights[MAX_NUM_LIGHTS];
};


layout(std430, binding = 2) readonly buffer Materials {
    Material materials[];
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


uniform int gNumLights = 0;
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
uniform vec4 gColorMod = vec4(1.0);
uniform vec4 gColorAdd = vec4(0);
uniform float gRimLightPower = 2.0;
uniform bool gRimLightEnabled = false;
uniform bool gCellShadingEnabled = false;
uniform bool gEnableSpecularExponent = false;
uniform bool gIsPBR = false;
uniform PBRMaterial gPBRmaterial;
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
        return materials[MaterialIndex].Color.AmbientColor;
    } else {
        return gMaterial.AmbientColor;
    }
}


vec4 GetMaterialDiffuseColor()
{
    if (gIsIndirectRender) {
        return materials[MaterialIndex].Color.DiffuseColor;
    } else {
        return gMaterial.DiffuseColor;
    }
}


vec4 GetMaterialSpecularColor()
{
    if (gIsIndirectRender) {
        return materials[MaterialIndex].Color.SpecularColor;
    } else {
        return gMaterial.SpecularColor;
    }
}

float CalcRimLightFactor(vec3 PixelToCamera, vec3 Normal)
{
    float RimFactor = dot(PixelToCamera, Normal);
    RimFactor = 1.0 - RimFactor;
    RimFactor = max(0.0, RimFactor);
    RimFactor = pow(RimFactor, gRimLightPower);
    return RimFactor;
}


float CalcShadowFactorPointLight(vec3 LightToPixel)
{
    float Distance = length(LightToPixel);

    LightToPixel.y = -LightToPixel.y;

    float SampledDistance = texture(gShadowCubeMap, LightToPixel).r;

    float bias = 0.015;

    if (SampledDistance + bias < Distance)
        return 0.05;
    else
        return 1.0;
}


vec3 CalcShadowCoords()
{
    vec3 ProjCoords = LightSpacePos0.xyz / LightSpacePos0.w;
    vec3 ShadowCoords = ProjCoords * 0.5 + vec3(0.5);
    return ShadowCoords;
}


float CalcShadowFactorBasic(vec3 LightDirection, vec3 Normal)
{
    vec3 ShadowCoords = CalcShadowCoords();

    float Depth = texture(gShadowMap, ShadowCoords.xy).x;

    float DiffuseFactor = dot(Normal, -LightDirection);

    float bias = max(0.05 * (1.0 - DiffuseFactor), 0.005);  

    if (Depth + bias < ShadowCoords.z)
        return 0.05;
    else
        return 1.0;
}


float CalcShadowFactorPCF(vec3 LightDirection, vec3 Normal)
{
    if (gShadowMapWidth == 0 || gShadowMapHeight == 0) {
        return 1.0;
    }

    vec3 ProjCoords = LightSpacePos0.xyz / LightSpacePos0.w;
    vec3 ShadowCoords = ProjCoords * 0.5 + vec3(0.5);

    float DiffuseFactor = dot(Normal, -LightDirection);
    float bias = mix(0.001, 0.0, DiffuseFactor);

    float TexelWidth = 1.0 / gShadowMapWidth;
    float TexelHeight = 1.0 / gShadowMapHeight;

    vec2 TexelSize = vec2(TexelWidth, TexelHeight);

    float ShadowSum = 0.0;

    int HalfFilterSize = gShadowMapFilterSize / 2;

    for (int y = -HalfFilterSize ; y < -HalfFilterSize + gShadowMapFilterSize ; y++) {
        for (int x = -HalfFilterSize ; x < -HalfFilterSize + gShadowMapFilterSize ; x++) {
            vec2 Offset = vec2(x, y) * TexelSize;
            float Depth = texture(gShadowMap, ShadowCoords.xy + Offset).x;

            if (Depth + bias < ShadowCoords.z) {
                ShadowSum += 0.0;
            } else {
                ShadowSum += 1.0;
            }
       }
   }

   float FinalShadowFactor = ShadowSum / float(pow(gShadowMapFilterSize, 2));

   return FinalShadowFactor;
}


float CalcShadowFactorWithRandomSampling(vec3 LightDirection, vec3 Normal)
{
    ivec3 OffsetCoord;
    vec2 f = mod(gl_FragCoord.xy, vec2(gShadowMapOffsetTextureSize));
    OffsetCoord.yz = ivec2(f);
    float Sum = 0.0;
    int SamplesDiv2 = int(gShadowMapOffsetFilterSize * gShadowMapOffsetFilterSize / 2.0);
    vec3 ShadowCoords = CalcShadowCoords();
    vec4 sc = vec4(ShadowCoords, 1.0);

    float TexelWidth = 1.0 / gShadowMapWidth;
    float TexelHeight = 1.0 / gShadowMapHeight;

    vec2 TexelSize = vec2(TexelWidth, TexelHeight);

    float DiffuseFactor = dot(Normal, -LightDirection);
    float bias = mix(0.001, 0.0, DiffuseFactor);
    float Depth = 0.0;

    for (int i = 0 ; i < 4 ; i++) {
        OffsetCoord.x = i;
        vec4 Offsets = texelFetch(gShadowMapOffsetTexture, OffsetCoord, 0) * gShadowMapRandomRadius;
        sc.xy = ShadowCoords.xy + Offsets.rg * TexelSize;
        Depth = texture(gShadowMap, sc.xy).x;
        if (Depth + bias < ShadowCoords.z) {
           Sum += 0.0;
        } else {
           Sum += 1.0;
        }

        sc.xy = ShadowCoords.xy + Offsets.ba * TexelSize;
        Depth = texture(gShadowMap, sc.xy).x;
        if (Depth + bias < ShadowCoords.z) {
           Sum += 0.0;
        } else {
           Sum += 1.0;
        }
    }

    float Shadow = Sum / 8.0;

    if (Shadow != 0.0 && Shadow != 1.0) {
        for (int i = 4 ; i < SamplesDiv2 ; i++) {
            OffsetCoord.x = i;
            vec4 Offsets = texelFetch(gShadowMapOffsetTexture, OffsetCoord, 0) * gShadowMapRandomRadius;
            sc.xy = ShadowCoords.xy + Offsets.rg * TexelSize;
            Depth = texture(gShadowMap, sc.xy).x;
            if (Depth + bias < ShadowCoords.z) {
               Sum += 0.0;
            } else {
               Sum += 1.0;
            }

            sc.xy = ShadowCoords.xy + Offsets.ba * TexelSize;
            Depth = texture(gShadowMap, sc.xy).x;
            if (Depth + bias < ShadowCoords.z) {
               Sum += 0.0;
            } else {
               Sum += 1.0;
            }
        }

        Shadow = Sum / float(SamplesDiv2 * 2.0);
    }

    return Shadow;
}


float CalcShadowFactor(vec3 LightDirection, vec3 Normal, bool IsPoint)
{
    float ShadowFactor = 1.0;

    if (gShadowsEnabled) {
        if (gShadowMapRandomRadius > 0.0) {
            ShadowFactor = CalcShadowFactorWithRandomSampling(LightDirection, Normal);        
        } else if (gShadowMapFilterSize > 0){
            ShadowFactor = CalcShadowFactorPCF(LightDirection, Normal);        
        } else {
            if (IsPoint) {
                ShadowFactor = CalcShadowFactorPointLight(LightDirection);            
            } else {
                ShadowFactor = CalcShadowFactorBasic(LightDirection, Normal);
            }
        }
    }

    return ShadowFactor;
}

vec4 CalcLightInternal(int Index, vec3 LightDirection, vec3 Normal, float ShadowFactor)
{
    vec4 AmbientColor = vec4(Lights[Index].Color, 1.0f) *
                        Lights[Index].AmbientIntensity *
                        GetMaterialAmbientColor();

    //return vec4(Lights[Index].AmbientIntensity);

    float DiffuseFactor = dot(Normal, -LightDirection);

    vec4 DiffuseColor = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);
    vec4 RimColor = vec4(0, 0, 0, 0);

    if (DiffuseFactor > 0) {
        if (gCellShadingEnabled) {
            DiffuseFactor = ceil(DiffuseFactor * toon_color_levels) * toon_scale_factor;
        }

        DiffuseColor = vec4(Lights[Index].Color, 1.0f) *
                       Lights[Index].DiffuseIntensity *
                       GetMaterialDiffuseColor() *
                       DiffuseFactor;

        vec3 PixelToCamera = normalize(gCameraWorldPos - WorldPos0);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(PixelToCamera, LightReflect);

        if (!gCellShadingEnabled && (SpecularFactor > 0)) {
            float SpecularExponent = 128.0;

            if (gEnableSpecularExponent) {
                SpecularExponent = texture(gSamplerSpecularExponent, TexCoord).r * 255.0;
            }

            SpecularFactor = pow(SpecularFactor, SpecularExponent);
            SpecularColor = vec4(Lights[Index].Color, 1.0f) *
                                 Lights[Index].DiffuseIntensity * // using the diffuse intensity for diffuse/specular
                                 GetMaterialSpecularColor() *
                                 SpecularFactor;
        }

        if (gRimLightEnabled) {
           float RimFactor = CalcRimLightFactor(PixelToCamera, Normal);
           RimColor = DiffuseColor * RimFactor;
        }
    }

    if (gHasNormalMap) {
        float OriginalDiffuse = dot(normalize(Normal0), -LightDirection);
        DiffuseColor *= OriginalDiffuse;
    }

    return (AmbientColor + ShadowFactor * (DiffuseColor + SpecularColor + RimColor));
   //return vec4(DiffuseFactor);
 // return DiffuseColor;
}


vec4 CalcDirectionalLight(int Index, vec3 Normal)
{
    float ShadowFactor = CalcShadowFactor(Lights[Index].Direction, Normal, false);
    //return vec4(ShadowFactor);
    return CalcLightInternal(Index, Lights[Index].Direction, Normal, ShadowFactor);
}


vec4 CalcPointLight(int Index, vec3 Normal, bool IsPoint)
{
    vec3 LightWorldDir = WorldPos0 - Lights[Index].WorldPos;
    float ShadowFactor = CalcShadowFactor(LightWorldDir, Normal, IsPoint);

    float Distance = length(LightWorldDir);
    LightWorldDir = normalize(LightWorldDir);
    vec4 Color = CalcLightInternal(Index, LightWorldDir, Normal, ShadowFactor);
    float Attenuation =  Lights[Index].Atten_Constant +
                         Lights[Index].Atten_Linear * Distance +
                         Lights[Index].Atten_Exp * Distance * Distance;

    return Color / Attenuation;
}


vec4 CalcSpotLight(int Index, vec3 Normal)
{
    vec3 PixelToLight = normalize(Lights[Index].WorldPos - WorldPos0);
    float SpotFactor = dot(PixelToLight, -Lights[Index].Direction);

    if (SpotFactor > Lights[Index].Cutoff) {
        vec4 Color = CalcPointLight(Index, Normal, false);
        float SpotLightIntensity = (1.0 - (1.0 - SpotFactor)/(1.0 - Lights[Index].Cutoff));
        return Color * SpotLightIntensity;
    }
    else {
        return vec4(0,0,0,0);
    }
}


float CalcLinearFogFactor()
{
    float CameraToPixelDist = length(WorldPos0 - gCameraWorldPos);
    float FogRange = gFogEnd - gFogStart;
    float FogDist = gFogEnd - CameraToPixelDist;
    float FogFactor = FogDist / FogRange;
    FogFactor = clamp(FogFactor, 0.0, 1.0);
    return FogFactor;
}


float CalcExpFogFactor()
{
    float CameraToPixelDist = length(WorldPos0 - gCameraWorldPos);
    float DistRatio = 4.0 * CameraToPixelDist / gFogEnd;
    float FogFactor = 1.0;

    if (gExpSquaredFogEnabled) {
        FogFactor = exp(-DistRatio * gExpFogDensity * DistRatio * gExpFogDensity);
    } else {
        FogFactor = exp(-DistRatio * gExpFogDensity);
    }

    return FogFactor;
}


float CalcLayeredFogFactor()
{
    vec3 CameraProj = gCameraWorldPos;
    CameraProj.y = 0.0;

    vec3 PixelProj = WorldPos0;
    PixelProj.y = 0.0;

    float DeltaD = length(CameraProj - PixelProj) / gFogEnd;

    float DeltaY = 0.0f;
    float DensityIntegral = 0.0f;

    if (gCameraWorldPos.y > gLayeredFogTop) { // The camera is above the top of the fog
        if (WorldPos0.y < gLayeredFogTop) {   // The pixel is inside the fog
            DeltaY = (gLayeredFogTop - WorldPos0.y) / gLayeredFogTop;
            DensityIntegral = DeltaY * DeltaY * 0.5;
        }                                     // else - the pixel is above the fog - nothing to do
    } else {                                  // The camera is inside the fog
        if (WorldPos0.y < gLayeredFogTop) {   // The pixel is inside the fog
            DeltaY = abs(gCameraWorldPos.y - WorldPos0.y) / gLayeredFogTop;
            float DeltaCamera = (gLayeredFogTop - gCameraWorldPos.y) / gLayeredFogTop;
            float DensityIntegralCamera = DeltaCamera * DeltaCamera * 0.5;
            float DeltaPixel = (gLayeredFogTop - WorldPos0.y) / gLayeredFogTop;
            float DensityIntegralPixel = DeltaPixel * DeltaPixel * 0.5;
            DensityIntegral = abs(DensityIntegralCamera - DensityIntegralPixel);
        } else {                              // The pixel is above the fog
            DeltaY = (gLayeredFogTop - gCameraWorldPos.y) / gLayeredFogTop;
            DensityIntegral = DeltaY * DeltaY * 0.5;
        }
    }

    float FogDensity = 0.0;

    if (DeltaY != 0) {
        FogDensity = (sqrt(1.0 + ((DeltaD / DeltaY) * (DeltaD / DeltaY)))) * DensityIntegral;
    }

    float FogFactor = exp(-FogDensity);

    return FogFactor;
}


#define PI 3.1415926535897932384626433832795

float CalcAnimatedFogFactor()
{
    float CameraToPixelDist = length(WorldPos0 - gCameraWorldPos);

    float DistRatio = CameraToPixelDist / gFogEnd;

    float ExpFogFactor = exp(-DistRatio * gExpFogDensity);

    float x = WorldPos0.x / 20.0;
    float y = WorldPos0.y / 20.0;
    float z = WorldPos0.z / 20.0;

    float AnimFactor = -(1.0 +
                         0.5 * cos(5.0 * PI * z + gFogTime) +
                         0.2 * cos(7.0 * PI * (z + 0.1 * x)) +
                         0.2 * cos(5.0 * PI * (z - 0.05 * x)) +
                         0.1 * cos(PI * x) * cos(PI * z / 2.0));

    float FogFactor = ExpFogFactor + (CameraToPixelDist / gFogEnd) * AnimFactor;

    return FogFactor;
}


float CalcFogFactor()
{
    float FogFactor = 1.0;

    if (gFogTime > 0.0) {
       FogFactor = CalcAnimatedFogFactor();
    } else if (gLayeredFogTop > 0.0) {
        FogFactor = CalcLayeredFogFactor();
    } else if (gFogStart >= 0.0) {
        FogFactor = CalcLinearFogFactor();
    } else {
        FogFactor = CalcExpFogFactor();
    }

    return FogFactor;
}


vec3 CalcBumpedNormal()                                                                     
{             
    // Step #1: Sample a normal from the normal map
    vec3 BumpMapNormal;
    
    if (gIsIndirectRender) {
        BumpMapNormal = texture(materials[MaterialIndex].NormalMap, TexCoord.xy).xyz;
    } else {
        BumpMapNormal = texture(gNormalMap, TexCoord).xyz;
    }
    
    // Step #2: transform from [0,1] to [-1,1]
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0);

    // Step #3: re-normalize the tangent space base vector after interpolation
    vec3 Normal = normalize(Normal0);
    vec3 Tangent = normalize(Tangent0);
    vec3 Bitangent = normalize(Bitangent0);

    // Improvement: re-orthogonalize the TBN base vectors using the Gram-Schmidt process:
    //              (in this case the bitangent is calculated on the fly so we don't need
    //              to put it in the vertex buffer)
    // Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    // vec3 Bitangent = cross(Tangent, Normal);    
    
    mat3 TBN = mat3(Tangent, Bitangent, Normal);

    // Step #4: transform from tangent space to world space
    vec3 NewNormal = normalize(TBN * BumpMapNormal);

    return NewNormal;                                                                       
}            


vec3 GetNormal()
{
    vec3 Normal;    

    if (gHasNormalMap) {
        Normal = CalcBumpedNormal();
    } else {
        Normal = normalize(Normal0);
    }

    return Normal;
}


vec4 GetTotalLight(vec3 Normal)
{        
    vec4 TotalLight = vec4(0.0);

    for (int i = 0 ; i < gNumLights ; i++) {
        switch (Lights[i].LightType) {

            case LIGHT_TYPE_DIR:
                TotalLight += CalcDirectionalLight(i, Normal);
                break;

            case LIGHT_TYPE_POINT:
                TotalLight += CalcPointLight(i, Normal, true);
                break;

            case LIGHT_TYPE_SPOT:
                TotalLight += CalcSpotLight(i, Normal);
                break;
        }
    }

    return TotalLight;
}


vec4 ApplyRefRefract(vec4 FinalColor, vec3 Normal)
{
    vec3 CameraToPixel = normalize(WorldPos0 - gCameraWorldPos);

    vec3 ReflectionDir = normalize(reflect(CameraToPixel, Normal));
    vec4 ColorReflect = vec4(texture(gCubemapTexture, ReflectionDir).rgb, 1.0);

    vec3 RefractionDir = normalize(refract(CameraToPixel, Normal, gETA));    

    float F = ((1.0 - gETA) * (1.0 - gETA)) / 
              ((1.0 + gETA) * (1.0 + gETA));

    float Ratio = F + (1.0 - F) * pow((1.0 - dot(-CameraToPixel, Normal)), gFresnelPower);

    vec4 ColorRefract = vec4(texture(gCubemapTexture, RefractionDir).rgb, 1.0);

    vec4 ColorRefractReflect = mix(ColorRefract, ColorReflect, Ratio);

    FinalColor = mix(FinalColor, ColorRefractReflect, gMatToRefRefractFactor);

    return FinalColor;
}


vec4 GetTexColor()
{
    vec4 TexColor;

    if (gIsIndirectRender) {
        TexColor = texture(materials[MaterialIndex].DiffuseMap, TexCoord.xy);
    } else if (gHasSampler) {
        TexColor = texture(gSampler, TexCoord.xy);
    } else {
        TexColor = vec4(1.0, 0.0, 0.0, 1.0);
    }

    return TexColor;
}


vec4 CalcPhongLighting(vec3 Normal)
{
    vec4 TotalLight;
    
    if (gNumLights > 0) {
        TotalLight = GetTotalLight(Normal);
      //  FragColor = TotalLight;
      //  return;
    } else {
        TotalLight = vec4(1.0);
    }

    vec4 TexColor = GetTexColor();

    vec4 FinalColor = TexColor * TotalLight;

    if (gRefRefractEnabled) {
        FinalColor = ApplyRefRefract(FinalColor, Normal);
    }  

    return FinalColor;
}


vec3 schlickFresnel(float vDotH, vec3 Albedo)
{
    vec3 F0 = vec3(0.04);    
    
    if (gPBRmaterial.IsAlbedo) {
        float Metallic = texture(gMetallic, TexCoord0.xy).x;
        F0 = mix(F0, Albedo, Metallic);
    } else {
        if (gPBRmaterial.IsMetal) {
	        F0 = gPBRmaterial.Color;
	    }
    }

    vec3 ret = F0 + (1 - F0) * pow(clamp(1.0 - vDotH, 0.0, 1.0), 5);

    return ret;
}


float GetRoughness()
{
    if (gPBRmaterial.IsAlbedo) {
        return texture(gRoughness, TexCoord0.xy).x;
    } else {
        return gPBRmaterial.Roughness;
    }
}


float geomSmith(float dp, float Roughness)
{
    float k = (Roughness + 1.0) * (Roughness + 1.0) / 8.0;
    float denom = dp * (1 - k) + k;
    return dp / denom;
}


float ggxDistribution(float nDotH, float Roughness)
{
    float alpha2 = Roughness * Roughness * Roughness * Roughness;
    float d = nDotH * nDotH * (alpha2 - 1) + 1;
    float ggxdistrib = alpha2 / (PI * d * d);
    return ggxdistrib;
}


// TODO: currently unused
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(gNormalMap, TexCoord0).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos0);
    vec3 Q2  = dFdy(WorldPos0);
    vec2 st1 = dFdx(TexCoord0);
    vec2 st2 = dFdy(TexCoord0);

    vec3 N   = normalize(Normal0);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}



vec3 CalcPBRLighting(int Index, vec3 PosDir, bool IsDirLight, vec3 Normal)
{
    vec3 LightIntensity = Lights[Index].Color * Lights[Index].DiffuseIntensity;

    vec3 l = vec3(0.0);

    if (IsDirLight) {
        l = -PosDir.xyz;
    } else {
        l = PosDir - WorldPos0;
        float LightToPixelDist = length(l);
        l = normalize(l);
        LightIntensity /= (LightToPixelDist * LightToPixelDist);
    }

    vec3 n = Normal;
    vec3 v = normalize(gCameraWorldPos - WorldPos0);
    vec3 h = normalize(v + l);

    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    vec3 fLambert = vec3(0.0);

    if (!gPBRmaterial.IsMetal) {
        if (gPBRmaterial.IsAlbedo) {
            fLambert = pow(texture(gAlbedo, TexCoord0.xy).xyz, vec3(2.2));
        } else {
            fLambert = gPBRmaterial.Color;
        }
    }

    vec3 F = schlickFresnel(vDotH, fLambert);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;

    float Roughness = GetRoughness();

    vec3 SpecBRDF_nom  = ggxDistribution(nDotH, Roughness) *
                         F *
                         geomSmith(nDotL, Roughness) *
                         geomSmith(nDotV, Roughness);

    float SpecBRDF_denom = 4.0 * nDotV * nDotL + 0.0001;

    vec3 SpecBRDF = SpecBRDF_nom / SpecBRDF_denom;

    vec3 DiffuseBRDF = kD * fLambert / PI;

    vec3 FinalColor = (DiffuseBRDF + SpecBRDF) * LightIntensity * nDotL;

    return FinalColor;
}


vec3 CalcPBRDirectionalLight(int Index, vec3 Normal)
{
    return CalcPBRLighting(Index, Lights[Index].Direction, true, Normal);
}


vec3 CalcPBRPointLight(int Index, vec3 Normal)
{
    return CalcPBRLighting(Index, Lights[Index].WorldPos, false, Normal);
}


vec4 CalcTotalPBRLighting(vec3 Normal)
{
    vec3 TotalLight = vec3(0.0);

    for (int i = 0 ; i < gNumLights ; i++) {
        switch (Lights[i].LightType) {
            case LIGHT_TYPE_DIR:
                TotalLight += CalcPBRDirectionalLight(i, Normal);
                break;
            case LIGHT_TYPE_POINT:
                TotalLight += CalcPBRPointLight(i, Normal);
                break;
        }
    }

    // HDR tone mapping
    TotalLight = TotalLight / (TotalLight + vec3(1.0));

    // Gamma correction
    vec4 FinalLight = vec4(pow(TotalLight, vec3(1.0/2.2)), 1.0);

    return FinalLight;
}


void main()
{
    TexCoord = TexCoord0;
    vec3 Normal = GetNormal();
    //FragColor = vec4(GetNormal(), 0.0);
    //FragColor = vec4(TexCoord, 0.0, 0.0);
    //return;
    
    if (gIsPBR) {
        FragColor = CalcTotalPBRLighting(Normal);
      //  FragColor = texture(gRoughness, TexCoord0.xy);
    } else {
        FragColor = CalcPhongLighting(Normal);
    }

    vec4 TempColor = vec4(0.0);

    if (gFogColor == vec3(0)) {
        TempColor = FragColor;
    } else {
        float FogFactor = CalcFogFactor();
        TempColor = mix(vec4(gFogColor, 1.0), FragColor, FogFactor);
    }
    
    // I'm using gColorMod and gColorAdd to enhance the color in
    // my youtube thumbnails. They are not an integral part of the lighting equation.
     FragColor = TempColor * gColorMod + gColorAdd;

   // FragColor = vec4(1.0);  
 // FragColor = GetTexColor();
  //  FragColor = GetTotalLight(Normal);
  //  FragColor = vec4(gDirectionalLight.Base.AmbientIntensity);
//    FragColor = texture(gSampler, TexCoord.xy);
   //   FragColor = texture(materials[MaterialIndex].DiffuseMap, TexCoord.xy);
    //FragColor = texture(gHeightMap, TexCoord0);

  //  FragColor = GetMaterialAmbientColor();
  //FragColor = vec4(Normal0, 1.0);
  // FragColor = vec4(Lights[1].DiffuseIntensity);
}
