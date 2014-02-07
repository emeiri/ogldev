#version 330

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices = 18) out;

in vec3 WorldPos[];

uniform vec3 gLightPos;
uniform mat4 gVP;

float EPSILON = 0.01;

void EmitQuad(int StartIndex, vec3 StartVertex, int EndIndex, vec3 EndVertex)
{
    vec3 LightDir = normalize(StartVertex - gLightPos);
    vec3 l = LightDir * EPSILON;
    gl_Position = gVP * vec4((StartVertex + l), 1.0);
    EmitVertex();
    
    gl_Position = gVP * vec4(LightDir, 0.0);
    EmitVertex();

    LightDir = normalize(EndVertex - gLightPos);
    l = LightDir * EPSILON;
    gl_Position = gVP * vec4((EndVertex + l), 1.0);
    EmitVertex();
    
    gl_Position = gVP * vec4(LightDir, 0.0);
    EmitVertex();

    EndPrimitive();            
}


void main()
{
    vec3 e1 = WorldPos[2] - WorldPos[0];
    vec3 e2 = WorldPos[4] - WorldPos[0];
    vec3 e3 = WorldPos[1] - WorldPos[0];
    vec3 e4 = WorldPos[3] - WorldPos[2];
    vec3 e5 = WorldPos[4] - WorldPos[2];
    vec3 e6 = WorldPos[5] - WorldPos[0];

    vec3 Normal = cross(e1,e2);
    vec3 LightDir = gLightPos - WorldPos[0];

    if (dot(Normal, LightDir) > 0.000001) {

        Normal = cross(e3,e1);

        if (dot(Normal, LightDir) <= 0) {
            vec3 StartVertex = WorldPos[0];
            vec3 EndVertex = WorldPos[2];
            EmitQuad(0, StartVertex, 2, EndVertex);
        }

        Normal = cross(e4,e5);
        LightDir = gLightPos - WorldPos[2];

        if (dot(Normal, LightDir) <= 0) {
            vec3 StartVertex = WorldPos[2];
            vec3 EndVertex = WorldPos[4];
            EmitQuad(2, StartVertex, 4, EndVertex);
        }

        Normal = cross(e2,e6);
        LightDir = gLightPos - WorldPos[4];

        if (dot(Normal, LightDir) <= 0) {
            vec3 StartVertex = WorldPos[4];
            vec3 EndVertex = WorldPos[0];
            EmitQuad(4, StartVertex, 0, EndVertex);
        }

        vec3 LightDir = (normalize(WorldPos[0] - gLightPos)) * EPSILON;
        gl_Position = gVP * vec4((WorldPos[0] + LightDir), 1.0);
        EmitVertex();

        LightDir = (normalize(WorldPos[2] - gLightPos)) * EPSILON;
        gl_Position = gVP * vec4((WorldPos[2] + LightDir), 1.0);
        EmitVertex();

        LightDir = (normalize(WorldPos[4] - gLightPos)) * EPSILON;
        gl_Position = gVP * vec4((WorldPos[4] + LightDir), 1.0);
        EmitVertex();
        EndPrimitive();
    }
}