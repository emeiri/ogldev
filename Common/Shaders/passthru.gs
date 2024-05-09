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
noperspective out vec3 EdgeDistance0; // to match wireframe_on_mesh.gs
                                                                                   
void main()                                                                         
{     
    EdgeDistance0 = vec3(-1.0, -1.0, -1.0);   // used only by wireframe_on_mesh.gs

    gl_Position = gl_in[0].gl_Position;
    gl_ClipDistance = gl_in[0].gl_ClipDistance;
    TexCoord0 = VTexCoord[0];
    Normal0 = VNormal[0];
    LocalPos0 = VLocalPos[0];
    WorldPos0 = VWorldPos[0];
    LightSpacePos0 = VLightSpacePos[0];
    EmitVertex();
                  
    gl_Position = gl_in[1].gl_Position;
    gl_ClipDistance = gl_in[1].gl_ClipDistance;
    TexCoord0 = VTexCoord[1];
    Normal0 = VNormal[1];
    LocalPos0 = VLocalPos[1];
    WorldPos0 = VWorldPos[1];
    LightSpacePos0 = VLightSpacePos[1];
    EmitVertex();                                                                   
    
    gl_Position = gl_in[2].gl_Position;
    gl_ClipDistance = gl_in[2].gl_ClipDistance;
    TexCoord0 = VTexCoord[2];
    Normal0 = VNormal[2];
    LocalPos0 = VLocalPos[2];
    WorldPos0 = VWorldPos[2];
    LightSpacePos0 = VLightSpacePos[2];
    EmitVertex();
                                                                                    
    EndPrimitive();                                                                 
}                                                                                   
