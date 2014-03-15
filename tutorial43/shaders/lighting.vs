interface VSOutput
{                                                                                    
    vec2 TexCoord;
    vec3 Normal;
    vec3 WorldPos;
};

struct VSOutput1
{
    vec2 TexCoord;
    vec3 Normal;
    vec3 WorldPos;
};

uniform mat4 gWVP;
uniform mat4 gWorld;

shader VSmain(in vec3 Position, in vec2 TexCoord, in vec3 Normal, out VSOutput VSout)
{
    gl_Position    = gWVP * vec4(Position, 1.0);
    VSout.TexCoord = TexCoord;
    VSout.Normal   = (gWorld * vec4(Normal, 0.0)).xyz;
    VSout.WorldPos = (gWorld * vec4(Position, 1.0)).xyz;
}

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

vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, VSOutput1 In, float ShadowFactor)           
{                                                                                           
    vec4 AmbientColor = vec4(Light.Color, 1.0f) * Light.AmbientIntensity;                   
    float DiffuseFactor = dot(In.Normal, -LightDirection);                                     
                                                                                            
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  
                                                                                            
    if (DiffuseFactor > 0) {                                                                
        DiffuseColor = vec4(Light.Color, 1.0f) * Light.DiffuseIntensity * DiffuseFactor;    
                                                                                            
        vec3 VertexToEye = normalize(gEyeWorldPos - In.WorldPos);                             
        vec3 LightReflect = normalize(reflect(LightDirection, In.Normal));                     
        float SpecularFactor = dot(VertexToEye, LightReflect);                              
        SpecularFactor = pow(SpecularFactor, gSpecularPower);                               
        if (SpecularFactor > 0) {                                                           
            SpecularColor = vec4(Light.Color, 1.0f) *                                       
                            gMatSpecularIntensity * SpecularFactor;                         
        }                                                                                   
    }                                                                                       
                                                                                            
    return (AmbientColor + ShadowFactor * (DiffuseColor + SpecularColor));                                   
}                                                                                           
                                                                                            

float CalcShadowFactor(vec3 LightDirection)
{
    float SampledDistance = texture(gShadowMap, LightDirection).r;

    float Distance = length(LightDirection);

    if (Distance <= SampledDistance + EPSILON)
        return 1.0;
    else
        return 0.0;
}
                                                                                            
vec4 CalcPointLight(PointLight l, VSOutput1 In)                       
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
                                                                                            
                                                                                            
shader FSmain(in VSOutput FSin, out vec4 FragColor)
{                                    
    VSOutput1 In;
    In.TexCoord     = FSin.TexCoord;
    In.Normal       = normalize(FSin.Normal);
    In.WorldPos     = FSin.WorldPos;                                                                 
  
    vec4 TotalLight = CalcPointLight(gPointLight, In);                                         
                                                                                            
	vec4 SampledColor = texture(gColorMap, In.TexCoord.xy);
                                                                                            
    FragColor = SampledColor * TotalLight;     
}

program ShadowsPCF
{
    vs(330)=VSmain();
    fs(330)=FSmain();
};