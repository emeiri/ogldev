#version 330
                                                                        
in vec3 ViewPos;

layout (location = 0) out vec3 PosOut;   
																			
void main()									
{											
    PosOut = ViewPos;
}
