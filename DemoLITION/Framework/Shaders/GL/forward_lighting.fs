#version 420

const int MAX_POINT_LIGHTS = 2;
const int MAX_SPOT_LIGHTS = 2;

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 WorldPos0;
in vec4 LightSpacePos;

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
    vec3 WorldPos;
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

uniform DirectionalLight gDirectionalLight;
uniform int gNumPointLights;
uniform PointLight gPointLights[MAX_POINT_LIGHTS];
uniform int gNumSpotLights;
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];
uniform Material gMaterial;
uniform bool gHasSampler = false;
layout(binding = 0) uniform sampler2D gSampler;
layout(binding = 1) uniform sampler2D gSamplerSpecularExponent;
layout(binding = 2) uniform sampler2D gShadowMap;        // required only for shadow mapping (spot/directional light)
layout(binding = 3) uniform samplerCube gShadowCubeMap;  // required only for shadow mapping (point light)
layout(binding = 4) uniform sampler3D gShadowMapOffsetTexture;
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

    LightToPixel.x = -LightToPixel.x;       // what???

    float SampledDistance = texture(gShadowCubeMap, LightToPixel).r;

    float bias = 0.015;

    if (SampledDistance + bias < Distance)
        return 0.05;
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

    float bias = 0.15;//max(0.05 * (1.0 - DiffuseFactor), 0.005);  

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


float CalcShadowFactor(vec3 LightDirection, vec3 Normal, bool IsPoint)
{
    float ShadowFactor = 0.0;

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

        vec3 PixelToCamera = normalize(gCameraWorldPos - WorldPos0);
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
    float ShadowFactor = CalcShadowFactor(gDirectionalLight.Direction, Normal, false);
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal, ShadowFactor);
}


vec4 CalcPointLight(PointLight l, vec3 Normal, bool IsPoint)
{
    vec3 LightWorldDir = WorldPos0 - l.WorldPos;
    float ShadowFactor = CalcShadowFactor(LightWorldDir, Normal, IsPoint);

    float Distance = length(LightWorldDir);
    LightWorldDir = normalize(LightWorldDir);
    vec4 Color = CalcLightInternal(l.Base, LightWorldDir, Normal, ShadowFactor);
    float Attenuation =  l.Atten.Constant +
                         l.Atten.Linear * Distance +
                         l.Atten.Exp * Distance * Distance;
Attenuation = 1.0;      // TODO: how to import correctly from blender
    return Color / Attenuation;
}


vec4 CalcSpotLight(SpotLight l, vec3 Normal)
{
    vec3 LightToPixel = normalize(WorldPos0 - l.Base.WorldPos);
    float SpotFactor = dot(LightToPixel, -l.Direction);             // TODO: why negative direction?

    if (SpotFactor > l.Cutoff) {
        vec4 Color = CalcPointLight(l.Base, Normal, false);
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


vec4 GetTotalLight()
{
    vec3 Normal = normalize(Normal0);
    vec4 TotalLight = CalcDirectionalLight(Normal);

    for (int i = 0 ;i < gNumPointLights ;i++) {
        TotalLight += CalcPointLight(gPointLights[i], Normal, true);
    }

    for (int i = 0 ;i < gNumSpotLights ;i++) {
        TotalLight += CalcSpotLight(gSpotLights[i], Normal);
    }

    return TotalLight;
}


void main()
{
    vec4 TotalLight = GetTotalLight();

    vec4 TexColor;

    if (gHasSampler) {
        TexColor = texture2D(gSampler, TexCoord0.xy);
    } else {
        TexColor = vec4(1.0);
    }

    TexColor *= TotalLight;

    vec4 TempColor = vec4(0.0);

    if (gFogColor == vec3(0)) {
        TempColor = TexColor;
    } else {
        float FogFactor = CalcFogFactor();
        TempColor = mix(vec4(gFogColor, 1.0), TexColor, FogFactor);
    }

    // I'm using gColorMod and gColorAdd to enhance the color in
    // my youtube thumbnails. They are not an integral part of the lighting equation.
    FragColor = TempColor * gColorMod + gColorAdd;
    //FragColor = texture2D(gSampler, TexCoord0.xy);
    //FragColor = TotalLight;
}
