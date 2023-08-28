#version 330                                                                        
                                                                                    
uniform sampler2D gColorMap;                                                        
                                                                                    
in vec2 TexCoord;                                                                   
out vec4 FragColor;                                                                 
                                                                                    
void main()                                                                         
{                                                                                   
    FragColor = texture2D(gColorMap, TexCoord);                                     
                                                                                    
    if (FragColor.r == 1 && FragColor.g == 1 && FragColor.b == 1) {
        discard;                                                                    
    }                                                                               
}
