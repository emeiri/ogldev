#version 330

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 WorldPos0;

struct VSOutput
{
    vec2 TexCoord;
    vec3 Normal;
    vec3 WorldPos;
};

struct BaseLight
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
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
    vec3 Position;                                                                          
    Attenuation Atten;                                                                      
};                                                                                          
                                                                                            
                                                                                           
uniform PointLight gPointLight;                                          
uniform sampler2D gColorMap;                                                                
uniform samplerCube gShadowMap;
uniform vec3 gEyeWorldPos;
uniform float gMatSpecularIntensity;
uniform float gSpecularPower;
uniform vec2 gMapSize;

#define EPSILON 0.00001

float CalcShadowFactor(vec3 LightDirection)
{
    float SampledDistance = texture(gShadowMap, LightDirection).r;

    float Distance = length(LightDirection);

    if (Distance <= SampledDistance + EPSILON)
        return 1.0;
    else
        return 0.5;
}   

                                                                                        
vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, VSOutput In, float ShadowFactor)           
{                                                                                           
    vec4 AmbientColor = vec4(Light.Color, 1.0) * Light.AmbientIntensity;                   
    float DiffuseFactor = dot(In.Normal, -LightDirection);                                     
                                                                                            
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  
                                                                                            
    if (DiffuseFactor > 0.0) {                                                                
        DiffuseColor = vec4(Light.Color, 1.0) * Light.DiffuseIntensity * DiffuseFactor;    
                                                                                            
        vec3 VertexToEye = normalize(gEyeWorldPos - In.WorldPos);                             
        vec3 LightReflect = normalize(reflect(LightDirection, In.Normal));                     
        float SpecularFactor = dot(VertexToEye, LightReflect);                              
        SpecularFactor = pow(SpecularFactor, gSpecularPower);                               
        if (SpecularFactor > 0.0) {                                                           
            SpecularColor = vec4(Light.Color, 1.0) *                                       
                            gMatSpecularIntensity * SpecularFactor;                         
        }                                                                                   
    }                                                                                       
                                                                                            
    return (AmbientColor + ShadowFactor * (DiffuseColor + SpecularColor));                                   
}                                                                                           

                                                                                            
vec4 CalcPointLight(PointLight l, VSOutput In)                       
{                                                                                           
    vec3 LightDirection = In.WorldPos - l.Position;
    float Distance = length(LightDirection);    
    float ShadowFactor = CalcShadowFactor(LightDirection);
    LightDirection = normalize(LightDirection);                                                                                            
    vec4 Color = CalcLightInternal(l.Base, LightDirection, In, ShadowFactor);
    float Attenuation =  l.Atten.Constant +                                                 
                         l.Atten.Linear * Distance +                                        
                         l.Atten.Exp * Distance * Distance;  
                                                                                           
    return Color / Attenuation;                                                             
}                                                                                           
                                                                                            
out vec4 FragColor;
                                                                                            
void main()
{                                    
    VSOutput In;
    In.TexCoord      = TexCoord0;
    In.Normal        = normalize(Normal0);
    In.WorldPos      = WorldPos0;
  
    vec4 TotalLight = CalcPointLight(gPointLight, In);                                         
                                                                                            
    vec4 SampledColor = texture(gColorMap, TexCoord0.xy);
                                                                                            
    FragColor = SampledColor * TotalLight;     
}
