#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 TexCoord;                                             
layout (location = 2) in vec3 Normal;                                               

uniform mat4 gWVP;
uniform mat4 gWV;

out vec3 Pos0;                                        
out vec3 Normal0;                                                                   

void main()
{       
    gl_Position = gWVP * vec4(Position, 1.0);
    Pos0        = (gWV * vec4(Position, 1.0)).xyz;
    Normal0     = (gWV * vec4(Normal, 0.0)).xyz;    
}