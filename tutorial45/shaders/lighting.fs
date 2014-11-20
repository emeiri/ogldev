#version 330                                                                        
                                                                                                                                                                      
                                                                                    
out vec4 FragColor;                                                                 
                                                                                    
                                                                                            
uniform sampler2D gPositionMap;
uniform sampler2D gNormalMap;
uniform vec2 gScreenSize;                                                                                            
                                                                                            
vec2 CalcTexCoord()
{
    return gl_FragCoord.xy / gScreenSize;
}
                                                                             
                                                                                            
void main()                                                                                 
{                                                                                           
    vec2 TexCoord = CalcTexCoord();
    vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
    vec3 Normal = texture(gNormalMap, TexCoord).xyz;
                                                                                            
    FragColor = vec4(WorldPos, 1.0) + vec4(Normal, 1.0);
}