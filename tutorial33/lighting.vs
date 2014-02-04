#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 TexCoord;                                             
layout (location = 2) in vec3 Normal;                                               
layout (location = 3) in mat4 WVP;                                                  
layout (location = 7) in mat4 World;                                                
                                                                                    
out vec2 TexCoord0;                                                                 
out vec3 Normal0;                                                                   
out vec3 WorldPos0;                                                                 
flat out int InstanceID;                                                            
                                                                                    
void main()                                                                         
{                                                                                   
    gl_Position = WVP * vec4(Position, 1.0);                                        
    TexCoord0   = TexCoord;                                                         
    Normal0     = (World * vec4(Normal, 0.0)).xyz;                                  
    WorldPos0   = (World * vec4(Position, 1.0)).xyz;                                
    InstanceID = gl_InstanceID;                                                     
}