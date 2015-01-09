#version 330
                                                                        
in vec3 Pos0;
in vec3 Normal0;

layout (location = 0) out vec3 PosOut;   
layout (location = 1) out vec3 NormalOut;     
																			
void main()									
{											
    PosOut    = Pos0;
    NormalOut = normalize(Normal0);
}
