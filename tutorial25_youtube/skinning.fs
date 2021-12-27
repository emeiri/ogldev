#version 330

const int MAX_POINT_LIGHTS = 2;
const int MAX_SPOT_LIGHTS = 2;

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 LocalPos0;
flat in ivec4 BoneIDs0;
in vec4 Weights0;

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
    vec3 LocalPos;
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
uniform sampler2D gSampler;
uniform sampler2D gSamplerSpecularExponent;
uniform vec3 gCameraLocalPos;
uniform int gDisplayBoneIndex;

vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal)
{
    vec4 AmbientColor = vec4(Light.Color, 1.0f) *
                        Light.AmbientIntensity *
                        vec4(gMaterial.AmbientColor, 1.0f);

    float DiffuseFactor = dot(Normal, -LightDirection);

    vec4 DiffuseColor = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    if (DiffuseFactor > 0) {
        DiffuseColor = vec4(Light.Color, 1.0f) *
                       Light.DiffuseIntensity *
                       vec4(gMaterial.DiffuseColor, 1.0f) *
                       DiffuseFactor;

        vec3 PixelToCamera = normalize(gCameraLocalPos - LocalPos0);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(PixelToCamera, LightReflect);
        if (SpecularFactor > 0) {
            float SpecularExponent = texture2D(gSamplerSpecularExponent, TexCoord0).r * 255.0;
            SpecularFactor = pow(SpecularFactor, SpecularExponent);
            SpecularColor = vec4(Light.Color, 1.0f) *
                            Light.DiffuseIntensity * // using the diffuse intensity for diffuse/specular
                            vec4(gMaterial.SpecularColor, 1.0f) *
                            SpecularFactor;
        }
    }

    return (AmbientColor + DiffuseColor + SpecularColor);
}


vec4 CalcDirectionalLight(vec3 Normal)
{
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal);
}

vec4 CalcPointLight(PointLight l, vec3 Normal)
{
    vec3 LightDirection = LocalPos0 - l.LocalPos;
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);

    vec4 Color = CalcLightInternal(l.Base, LightDirection, Normal);
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
        vec4 Color = CalcPointLight(l.Base, Normal);
        float SpotLightIntensity = (1.0 - (1.0 - SpotFactor)/(1.0 - l.Cutoff));
        return Color * SpotLightIntensity * 1.5;
    }
    else {
        return vec4(0,0,0,0);
    }
}

void main()
{
    vec3 Normal = normalize(Normal0);
    vec4 TotalLight = CalcDirectionalLight(Normal);

    for (int i = 0 ;i < gNumPointLights ;i++) {
        TotalLight += CalcPointLight(gPointLights[i], Normal);
    }

    for (int i = 0 ;i < gNumSpotLights ;i++) {
        TotalLight += CalcSpotLight(gSpotLights[i], Normal);
    }

    bool found = false;

    for (int i = 0 ; i < 4 ; i++) {
        if (BoneIDs0[i] == gDisplayBoneIndex) {
           if (Weights0[i] >= 0.7) {
               FragColor = vec4(1.0, 0.0, 0.0, 0.0) * Weights0[i];
           } else if (Weights0[i] >= 0.4 && Weights0[i] <= 0.6) {
               FragColor = vec4(0.0, 1.0, 0.0, 0.0) * Weights0[i];
           } else if (Weights0[i] >= 0.1) {
               FragColor = vec4(1.0, 1.0, 0.0, 0.0) * Weights0[i];
           }

           found = true;
           break;
        }
    }

    if (!found ) {
         FragColor = texture2D(gSampler, TexCoord0.xy) * TotalLight * vec4(0.0001) + vec4(0.0, 0.0, 1.0, 0.0);
    }
}
