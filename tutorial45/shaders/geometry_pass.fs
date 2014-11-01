#version 330
                                                                        
in vec3 Normal0;
in vec3 WorldPos0;

layout (location = 0) out vec3 WorldPosOut;   
layout (location = 2) out vec3 NormalOut;     
																			
void main()									
{											
    WorldPosOut = WorldPos0;
    NormalOut   = normalize(Normal0);
}
