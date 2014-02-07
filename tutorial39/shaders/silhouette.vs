#version 330

layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 TexCoord;                                             
layout (location = 2) in vec3 Normal;                                               

out vec3 WorldPos0;                                                                 

uniform mat4 gWVP;                                                  
uniform mat4 gWorld;                                                
                                                                                    
void main()                                                                         
{                                                                                   
    vec4 PosL   = vec4(Position, 1.0);
    gl_Position = gWVP * PosL;
    WorldPos0   = (gWorld * PosL).xyz;                                
}
