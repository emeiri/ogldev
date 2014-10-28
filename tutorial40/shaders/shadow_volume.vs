#version 330

layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 TexCoord;                                             
layout (location = 2) in vec3 Normal;                                               

out vec3 PosL;
                                                                                    
void main()                                                                         
{                                                                                   
    PosL = Position;
}
