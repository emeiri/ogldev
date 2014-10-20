#version 330

layout (triangles_adjacency) in;    // six vertices in
layout (triangle_strip, max_vertices = 18) out; // 4 per quad * 3 triangle vertices + 6 for near/far caps

in vec3 PosL[]; // an array of 6 vertices (triangle with adjacency)

uniform vec3 gLightPos;
uniform mat4 gWVP;

float EPSILON = 0.0001;

// Emit a quad using a triangle strip
void EmitQuad(vec3 StartVertex, vec3 EndVertex)
{    
    // Vertex #1: the starting vertex (just a tiny bit below the original edge)
    vec3 LightDir = normalize(StartVertex - gLightPos);   
    gl_Position = gWVP * vec4((StartVertex + LightDir * EPSILON), 1.0);
    EmitVertex();
 
    // Vertex #2: the starting vertex projected to infinity
    gl_Position = gWVP * vec4(LightDir, 0.0);
    EmitVertex();
    
    // Vertex #3: the ending vertex (just a tiny bit below the original edge)
    LightDir = normalize(EndVertex - gLightPos);
    gl_Position = gWVP * vec4((EndVertex + LightDir * EPSILON), 1.0);
    EmitVertex();
    
    // Vertex #4: the ending vertex projected to infinity
    gl_Position = gWVP * vec4(LightDir , 0.0);
    EmitVertex();

    EndPrimitive();            
}


void main()
{
    vec3 e1 = PosL[2] - PosL[0];
    vec3 e2 = PosL[4] - PosL[0];
    vec3 e3 = PosL[1] - PosL[0];
    vec3 e4 = PosL[3] - PosL[2];
    vec3 e5 = PosL[4] - PosL[2];
    vec3 e6 = PosL[5] - PosL[0];

    vec3 Normal = normalize(cross(e1,e2));
    vec3 LightDir = normalize(gLightPos - PosL[0]);

    // Handle only light facing triangles
    if (dot(Normal, LightDir) > 0) {

        Normal = cross(e3,e1);

        if (dot(Normal, LightDir) <= 0) {
            vec3 StartVertex = PosL[0];
            vec3 EndVertex = PosL[2];
            EmitQuad(StartVertex, EndVertex);
        }

        Normal = cross(e4,e5);
        LightDir = gLightPos - PosL[2];

        if (dot(Normal, LightDir) <= 0) {
            vec3 StartVertex = PosL[2];
            vec3 EndVertex = PosL[4];
            EmitQuad(StartVertex, EndVertex);
        }

        Normal = cross(e2,e6);
        LightDir = gLightPos - PosL[4];

        if (dot(Normal, LightDir) <= 0) {
            vec3 StartVertex = PosL[4];
            vec3 EndVertex = PosL[0];
            EmitQuad(StartVertex, EndVertex);
        }

        // render the front cap
        LightDir = (normalize(PosL[0] - gLightPos));
        gl_Position = gWVP * vec4((PosL[0] + LightDir * EPSILON), 1.0);
        EmitVertex();

        LightDir = (normalize(PosL[2] - gLightPos));
        gl_Position = gWVP * vec4((PosL[2] + LightDir * EPSILON), 1.0);
        EmitVertex();

        LightDir = (normalize(PosL[4] - gLightPos));
        gl_Position = gWVP * vec4((PosL[4] + LightDir * EPSILON), 1.0);
        EmitVertex();
        EndPrimitive();
 
        // render the back cap
        LightDir = PosL[0] - gLightPos;
        gl_Position = gWVP * vec4(LightDir, 0.0);
        EmitVertex();

        LightDir = PosL[4] - gLightPos;
        gl_Position = gWVP * vec4(LightDir, 0.0);
        EmitVertex();

        LightDir = PosL[2] - gLightPos;
        gl_Position = gWVP * vec4(LightDir, 0.0);
        EmitVertex();

        EndPrimitive();
    }
}