#version 330                                                                        
                                                                                    
layout(triangles) in;                                                                  
layout(triangle_strip) out;                                                         
layout(max_vertices = 3) out;                                                       
                         
in vec2 VTexCoord[];
in vec3 VNormal[];
in vec3 VLocalPos[];
in vec3 VWorldPos[];
in vec4 VLightSpacePos[]; // required only for shadow mapping (spot/directional light)

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LocalPos0;
out vec3 WorldPos0;
out vec4 LightSpacePos0; // required only for shadow mapping (spot/directional light)                       
noperspective out vec3 EdgeDistance0;

uniform mat4 gViewportMatrix;
                                                                                   
void main()                                                                         
{        
    vec4 p;

    p = gl_in[0].gl_Position;
    vec2 p0 = vec2(gViewportMatrix * (p / p.w));

    p = gl_in[1].gl_Position;
    vec2 p1 = vec2(gViewportMatrix * (p / p.w));

    p = gl_in[2].gl_Position;
    vec2 p2 = vec2(gViewportMatrix * (p / p.w));

    float a = length(p1 - p2);
    float b = length(p2 - p0);
    float c = length(p1 - p0);

    float alpha = acos((b * b + c * c - a * a) / (2.0 * b * c));
    float beta  = acos((a * a + c * c - b * b) / (2.0 * a * c));

    float ha = abs(c * sin(beta));
    float hb = abs(c * sin(alpha));
    float hc = abs(b * sin(alpha));

    gl_Position = gl_in[0].gl_Position;
    gl_ClipDistance = gl_in[0].gl_ClipDistance;
    TexCoord0 = VTexCoord[0];
    Normal0 = VNormal[0];
    LocalPos0 = VLocalPos[0];
    WorldPos0 = VWorldPos[0];
    LightSpacePos0 = VLightSpacePos[0];
    EdgeDistance0 = vec3(ha, 0.0, 0.0);
    EmitVertex();
                  
    gl_Position = gl_in[1].gl_Position;
    gl_ClipDistance = gl_in[1].gl_ClipDistance;
    TexCoord0 = VTexCoord[1];
    Normal0 = VNormal[1];
    LocalPos0 = VLocalPos[1];
    WorldPos0 = VWorldPos[1];
    LightSpacePos0 = VLightSpacePos[1];
    EdgeDistance0 = vec3(0.0, hb, 0.0);
    EmitVertex();                                                                   
    
    gl_Position = gl_in[2].gl_Position;
    gl_ClipDistance = gl_in[2].gl_ClipDistance;
    TexCoord0 = VTexCoord[2];
    Normal0 = VNormal[2];
    LocalPos0 = VLocalPos[2];
    WorldPos0 = VWorldPos[2];
    LightSpacePos0 = VLightSpacePos[2];
    EdgeDistance0 = vec3(0.0, 0.0, hc);
    EmitVertex();
                                                                                    
    EndPrimitive();                                                                 
}                                                                                   
