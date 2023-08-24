#version 330                                                                        

layout(triangles) in;                                                                  
layout(triangle_strip) out;                                                         
layout(max_vertices = 3) out;                                                       
                                                                                    
uniform mat4 gVP;                                                                   

in VS_OUT {
    vec2 Tex;
} gs_in[];


out vec2 Tex;
out vec3 Normal;

void main()                                                                         
{   
    vec3 Pos0 = gl_in[0].gl_Position.xyz;
    vec3 Pos1 = gl_in[1].gl_Position.xyz;
    vec3 Pos2 = gl_in[2].gl_Position.xyz;

    vec3 Edge1 = Pos1 - Pos0;
    vec3 Edge2 = Pos2 - Pos0;

    Normal = cross(Edge1, Edge2);
    
    gl_Position = gVP * vec4(Pos0, 1.0);
    Tex = gs_in[0].Tex;
    EmitVertex();

    
    gl_Position = gVP * vec4(Pos1, 1.0);
    Tex = gs_in[1].Tex;
    EmitVertex();

    
    gl_Position = gVP * vec4(Pos2, 1.0);
    Tex = gs_in[2].Tex;
    EmitVertex();

    EndPrimitive();
}
