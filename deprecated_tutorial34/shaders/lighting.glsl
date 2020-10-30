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
    gl_Position      = gWVP * vec4(Position, 1.0);                                        
    VSout.TexCoord   = TexCoord;                                                         
    VSout.Normal     = (gWorld * vec4(Normal, 0.0)).xyz;                                  
    VSout.WorldPos   = (gWorld * vec4(Position, 1.0)).xyz;                                
}
                                                                                 
const int MAX_POINT_LIGHTS = 2;                                                     
const int MAX_SPOT_LIGHTS = 2;                                                      
                                                                                                                                                                      
                                                                                   
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
    vec3 Position;                                                                          
    Attenuation Atten;                                                                      
};                                                                                          
                                                                                            
struct SpotLight                                                                            
{                                                                                           
    PointLight Base;                                                                 
    vec3 Direction;                                                                         
    float Cutoff;                                                                           
};                                                                                          
                                                                                            
uniform int gNumPointLights;                                                                
uniform int gNumSpotLights;                                                                 
uniform DirectionalLight gDirectionalLight;                                                 
uniform PointLight gPointLights[MAX_POINT_LIGHTS];                                          
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];                                             
uniform sampler2D gColorMap;                                                                
uniform vec3 gEyeWorldPos;                                                                  
uniform float gMatSpecularIntensity;                                                        
uniform float gSpecularPower;                                                               
uniform vec4 gColor;                                                                     
                                                                                            
vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, VSOutput1 In)            
{                                                                                           
    vec4 AmbientColor = vec4(Light.Color * Light.AmbientIntensity, 1.0f);
    float DiffuseFactor = dot(In.Normal, -LightDirection);                                     
                                                                                            
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  
                                                                                            
    if (DiffuseFactor > 0) {                                                                
        DiffuseColor = vec4(Light.Color * Light.DiffuseIntensity * DiffuseFactor, 1.0f);
                                                                                            
        vec3 VertexToEye = normalize(gEyeWorldPos - In.WorldPos);                             
        vec3 LightReflect = normalize(reflect(LightDirection, In.Normal));                     
        float SpecularFactor = dot(VertexToEye, LightReflect);                                      
        if (SpecularFactor > 0) {                                                           
            SpecularFactor = pow(SpecularFactor, gSpecularPower);                               
            SpecularColor = vec4(Light.Color * gMatSpecularIntensity * SpecularFactor, 1.0f);
        }                                                                                   
    }                                                                                       
                                                                                            
    return (AmbientColor + DiffuseColor + SpecularColor);                                   
}                                                                                           
                                                                                            
vec4 CalcDirectionalLight(VSOutput1 In)                                                      
{                                                                                           
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, In);  
}                                                                                           
                                                                                            
vec4 CalcPointLight(PointLight l, VSOutput1 In)                                       
{                                                                                           
    vec3 LightDirection = In.WorldPos - l.Position;                                           
    float Distance = length(LightDirection);                                                
    LightDirection = normalize(LightDirection);                                             
                                                                                            
    vec4 Color = CalcLightInternal(l.Base, LightDirection, In);                         
    float Attenuation =  l.Atten.Constant +                                                 
                         l.Atten.Linear * Distance +                                        
                         l.Atten.Exp * Distance * Distance;                                 
                                                                                            
    return Color / Attenuation;                                                             
}                                                                                           
                                                                                            
vec4 CalcSpotLight(SpotLight l, VSOutput1 In)                                         
{                                                                                           
    vec3 LightToPixel = normalize(In.WorldPos - l.Base.Position);                             
    float SpotFactor = dot(LightToPixel, l.Direction);                                      
                                                                                            
    if (SpotFactor > l.Cutoff) {                                                            
        vec4 Color = CalcPointLight(l.Base, In);                                        
        return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - l.Cutoff));                   
    }                                                                                       
    else {                                                                                  
        return vec4(0,0,0,0);                                                               
    }                                                                                       
}                                                                                           
                                                                                            
shader FSmain(in VSOutput FSin, out vec4 FragColor)
{                                    
    VSOutput1 In;
    In.TexCoord = FSin.TexCoord;
    In.Normal = normalize(FSin.Normal);
    In.WorldPos = FSin.WorldPos;                                                                 
  
    vec4 TotalLight = CalcDirectionalLight(In);                                         
                                                                                            
    for (int i = 0 ; i < gNumPointLights ; i++) {                                           
        TotalLight += CalcPointLight(gPointLights[i], In);                              
    }                                                                                       
                                                                                            
    for (int i = 0 ; i < gNumSpotLights ; i++) {                                            
        TotalLight += CalcSpotLight(gSpotLights[i], In);                                
    }                                                                                       
                                                                                            
    FragColor = texture(gColorMap, In.TexCoord.xy) * TotalLight + gColor;     
}

program Lighting
{
    vs(330)=VSmain();
    fs(330)=FSmain();
};