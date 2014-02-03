#version 330                                                                        
                                                                                                                                                                                                                                                        
uniform unsigned int gDrawIndex;                                                            
uniform unsigned int gObjectIndex;                                                          
                                                                                    
out vec3 FragColor;

void main()                                                                         
{                                                                                   
    FragColor = uvec3(gObjectIndex, gDrawIndex,gl_PrimitiveID + 1);                 
}