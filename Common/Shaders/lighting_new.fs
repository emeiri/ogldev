#version 420

const int MAX_POINT_LIGHTS = 2;
const int MAX_SPOT_LIGHTS = 2;

in vec4 LightSpacePos; // required only for shadow mapping
in vec2 TexCoord0;
in vec3 Normal0;
in vec3 LocalPos0;
in vec3 WorldPos0;

out vec4 FragColor;

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
    vec3 LocalPos; // used for lighting calculations
    vec3 WorldPos; // used for point light shadow mapping
    Attenuation Atten;
};

struct SpotLight
{
    PointLight Base;
    vec3 Direction;
    float Cutoff;
};

struct Material
{
    vec3 AmbientColor;
    vec3 DiffuseColor;
    vec3 SpecularColor;
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
};

uniform DirectionalLight gDirectionalLight;
uniform int gNumPointLights;
uniform PointLight gPointLights[MAX_POINT_LIGHTS];
uniform int gNumSpotLights;
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];
uniform Material gMaterial;
layout(binding = 0) uniform sampler2D gSampler;
layout(binding = 1) uniform sampler2D gSamplerSpecularExponent;
layout(binding = 2) uniform sampler2D gShadowMap;        // required only for shadow mapping (spot/directional light)
layout(binding = 3) uniform samplerCube gShadowCubeMap;  // required only for shadow mapping (point light)
uniform int gShadowMapWidth = 0;
uniform int gShadowMapHeight = 0;
uniform int gShadowMapFilterSize = 0;
uniform sampler3D gShadowMapOffsetTexture;
uniform float gShadowMapOffsetTextureSize;
uniform float gShadowMapOffsetFilterSize;
uniform float gShadowMapRandomRadius = 0.0;
uniform vec3 gCameraLocalPos;
uniform vec3 gCameraWorldPos;
uniform vec4 gColorMod = vec4(1);
uniform vec4 gColorAdd = vec4(0);
uniform float gRimLightPower = 2.0;
uniform bool gRimLightEnabled = false;
uniform bool gCellShadingEnabled = false;
uniform bool gEnableSpecularExponent = false;
uniform bool gIsPBR = false;
uniform PBRMaterial gPBRmaterial;

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
        return 0.25;
    else
        return 1.0;
}


vec3 CalcShadowCoords()
{
    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;
    vec3 ShadowCoords = ProjCoords * 0.5 + vec3(0.5);
    return ShadowCoords;
}


float CalcShadowFactorBasic(vec3 LightDirection, vec3 Normal)
{
    vec3 ShadowCoords = CalcShadowCoords();

    float Depth = texture(gShadowMap, ShadowCoords.xy).x;

    float DiffuseFactor = dot(Normal, -LightDirection);

    float bias = mix(0.001, 0.0, DiffuseFactor);

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

    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;
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


float CalcShadowFactor(vec3 LightDirection, vec3 Normal)
{
    float ShadowFactor = 0.0;

    if (gShadowMapRandomRadius > 0.0) {
        ShadowFactor = CalcShadowFactorWithRandomSampling(LightDirection, Normal);
    } else if (gShadowMapFilterSize > 0){
        ShadowFactor = CalcShadowFactorPCF(LightDirection, Normal);
    } else {
        ShadowFactor = CalcShadowFactorBasic(LightDirection, Normal);
    }

    return ShadowFactor;
}

vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal,
                       float ShadowFactor)
{
    vec4 AmbientColor = vec4(Light.Color, 1.0f) *
                        Light.AmbientIntensity *
                        vec4(gMaterial.AmbientColor, 1.0f);

    float DiffuseFactor = dot(Normal, -LightDirection);

    vec4 DiffuseColor = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);
    vec4 RimColor = vec4(0, 0, 0, 0);

    if (DiffuseFactor > 0) {
        if (gCellShadingEnabled) {
            DiffuseFactor = ceil(DiffuseFactor * toon_color_levels) * toon_scale_factor;
        }

        DiffuseColor = vec4(Light.Color, 1.0f) *
                       Light.DiffuseIntensity *
                       vec4(gMaterial.DiffuseColor, 1.0f) *
                       DiffuseFactor;

        vec3 PixelToCamera = normalize(gCameraLocalPos - LocalPos0);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(PixelToCamera, LightReflect);

        if (!gCellShadingEnabled && (SpecularFactor > 0)) {
            float SpecularExponent = 128.0;

            if (gEnableSpecularExponent) {
                SpecularExponent = texture2D(gSamplerSpecularExponent, TexCoord0).r * 255.0;
            }

            SpecularFactor = pow(SpecularFactor, SpecularExponent);
            SpecularColor = vec4(Light.Color, 1.0f) *
                            Light.DiffuseIntensity * // using the diffuse intensity for diffuse/specular
                            vec4(gMaterial.SpecularColor, 1.0f) *
                            SpecularFactor;
        }

        if (gRimLightEnabled) {
           float RimFactor = CalcRimLightFactor(PixelToCamera, Normal);
           RimColor = DiffuseColor * RimFactor;
        }
    }

    return (AmbientColor + ShadowFactor * (DiffuseColor + SpecularColor + RimColor));
}


vec4 CalcDirectionalLight(vec3 Normal)
{
    float ShadowFactor = CalcShadowFactor(gDirectionalLight.Direction, Normal);
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal, ShadowFactor);
}


vec4 CalcPointLight(PointLight l, vec3 Normal, bool IsSpot)
{
    vec3 LightWorldDir = WorldPos0 - l.WorldPos;
    float ShadowFactor = 0.0;
    
    if (IsSpot) {
        ShadowFactor = CalcShadowFactor(LightWorldDir, Normal);
    } else {
        ShadowFactor = CalcShadowFactorPointLight(LightWorldDir);
    }

    vec3 LightLocalDir = LocalPos0 - l.LocalPos;
    float Distance = length(LightLocalDir);
    LightLocalDir = normalize(LightLocalDir);
    vec4 Color = CalcLightInternal(l.Base, LightLocalDir, Normal, ShadowFactor);
    float Attenuation =  l.Atten.Constant +
                         l.Atten.Linear * Distance +
                         l.Atten.Exp * Distance * Distance;

    return Color / Attenuation;
}


vec4 CalcSpotLight(SpotLight l, vec3 Normal)
{
    vec3 LightToPixel = normalize(LocalPos0 - l.Base.LocalPos);
    float SpotFactor = dot(LightToPixel, l.Direction);

    if (SpotFactor > l.Cutoff) {
        vec4 Color = CalcPointLight(l.Base, Normal, true);
        float SpotLightIntensity = (1.0 - (1.0 - SpotFactor)/(1.0 - l.Cutoff));
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


vec4 CalcPhongLighting()
{
    vec3 Normal = normalize(Normal0);
    vec4 TotalLight = CalcDirectionalLight(Normal);

    for (int i = 0 ;i < gNumPointLights ;i++) {
        TotalLight += CalcPointLight(gPointLights[i], Normal, false);
    }

    for (int i = 0 ;i < gNumSpotLights ;i++) {
        TotalLight += CalcSpotLight(gSpotLights[i], Normal);
    }

    vec4 TempColor = texture2D(gSampler, TexCoord0.xy) * TotalLight;

    if (gFogColor != vec3(0)) {
        float FogFactor = CalcFogFactor();

        TempColor = mix(vec4(gFogColor, 1.0), TempColor, FogFactor);
    }

    // I'm using gColorMod and gColorAdd to enhance the color in
    // my youtube thumbnails. They are not an integral part of the lighting equation.
    vec4 FinalColor =  TempColor * gColorMod + gColorAdd;

    return FinalColor;
}


vec3 schlickFresnel(float vDotH)
{
    vec3 F0 = vec3(0.04);

    if (gPBRmaterial.IsMetal) {
        F0 = gPBRmaterial.Color;
    }

    vec3 ret = F0 + (1 - F0) * pow(clamp(1.0 - vDotH, 0.0, 1.0), 5);

    return ret;
}


float geomSmith(float dp)
{
    float k = (gPBRmaterial.Roughness + 1.0) * (gPBRmaterial.Roughness + 1.0) / 8.0;
    float denom = dp * (1 - k) + k;
    return dp / denom;
}


float ggxDistribution(float nDotH)
{
    float alpha2 = gPBRmaterial.Roughness * gPBRmaterial.Roughness * gPBRmaterial.Roughness * gPBRmaterial.Roughness;
    float d = nDotH * nDotH * (alpha2 - 1) + 1;
    float ggxdistrib = alpha2 / (PI * d * d);
    return ggxdistrib;
}


vec3 CalcPBRLighting(BaseLight Light, vec3 PosDir, bool IsDirLight, vec3 Normal)
{
    vec3 LightIntensity = Light.Color * Light.DiffuseIntensity;

    vec3 l = vec3(0.0);

    if (IsDirLight) {
        l = -PosDir.xyz;
    } else {
        l = PosDir - LocalPos0;
        float LightToPixelDist = length(l);
        l = normalize(l);
        LightIntensity /= (LightToPixelDist * LightToPixelDist);
    }

    vec3 n = Normal;
    vec3 v = normalize(gCameraLocalPos - LocalPos0);
    vec3 h = normalize(v + l);

    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);

    vec3 F = schlickFresnel(vDotH);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;

    vec3 SpecBRDF_nom  = ggxDistribution(nDotH) *
                         F *
                         geomSmith(nDotL) *
                         geomSmith(nDotV);

    float SpecBRDF_denom = 4.0 * nDotV * nDotL + 0.0001;

    vec3 SpecBRDF = SpecBRDF_nom / SpecBRDF_denom;

    vec3 fLambert = vec3(0.0);

    if (!gPBRmaterial.IsMetal) {
        fLambert = gPBRmaterial.Color;
    }

    vec3 DiffuseBRDF = kD * fLambert / PI;

    vec3 FinalColor = (DiffuseBRDF + SpecBRDF) * LightIntensity * nDotL;

    return FinalColor;
}


vec3 CalcPBRDirectionalLight(vec3 Normal)
{
    return CalcPBRLighting(gDirectionalLight.Base, gDirectionalLight.Direction, true, Normal);
}


vec3 CalcPBRPointLight(PointLight l, vec3 Normal)
{
    return CalcPBRLighting(l.Base, l.LocalPos, false, Normal);
}


vec4 CalcTotalPBRLighting()
{
    vec3 Normal = normalize(Normal0);

    vec3 TotalLight = CalcPBRDirectionalLight(Normal);

    for (int i = 0 ;i < gNumPointLights ;i++) {
        TotalLight += CalcPBRPointLight(gPointLights[i], Normal);
    }

    // HDR tone mapping
    TotalLight = TotalLight / (TotalLight + vec3(1.0));

    // Gamma correction
    vec4 FinalLight = vec4(pow(TotalLight, vec3(1.0/2.2)), 1.0);

    return FinalLight;
}


void main()
{
    if (gIsPBR) {
        FragColor = CalcTotalPBRLighting();
    } else {
        FragColor = CalcPhongLighting();
    }
}
