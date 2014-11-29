#version 330
                                                                        
in vec3 WorldPos0;
in vec3 Normal0;

layout (location = 0) out vec3 WorldPosOut;   
layout (location = 1) out vec3 NormalOut;     
																			
void main()									
{											
    WorldPosOut = WorldPos0;
    NormalOut   = normalize(Normal0);
}
