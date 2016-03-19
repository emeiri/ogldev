#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;

const int NUM_CASCADES = 3;

uniform mat4 gWVP;
uniform mat4 gLightWVP[NUM_CASCADES];
uniform mat4 gWorld;

out vec4 LightSpacePos[NUM_CASCADES];
out float ClipSpacePosZ;
out vec2 TexCoord0;
out vec3 Normal0;
out vec3 WorldPos0;

void main()
{
    vec4 Pos = vec4(Position, 1.0);

    gl_Position = gWVP * Pos;
    
    for (int i = 0 ; i < NUM_CASCADES ; i++) {
        LightSpacePos[i] = gLightWVP[i] * Pos;
    }

    ClipSpacePosZ = gl_Position.z;
    TexCoord0     = TexCoord;
    Normal0       = (gWorld * vec4(Normal, 0.0)).xyz;
    WorldPos0     = (gWorld * vec4(Position, 1.0)).xyz;
}
