#version 330
                                                                        
in vec3 ViewPos;
in vec3 ViewNormal;

layout (location = 0) out vec3 PosOut;   
layout (location = 1) out vec3 NormalOut;     
																			
void main()									
{											
    PosOut    = ViewPos;
    NormalOut = normalize(ViewNormal);
}
