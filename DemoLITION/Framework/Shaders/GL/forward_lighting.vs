#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;

uniform mat4 gWVP;
uniform mat4 gLightWVP;
uniform mat4 gWorld;
uniform mat3 gNormalMatrix;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 WorldPos0;
out vec4 LightSpacePos;

void main()
{
    vec4 Pos4 = vec4(Position, 1.0);
    gl_Position = gWVP * Pos4;
    TexCoord0 = TexCoord;
    Normal0 = gNormalMatrix * Normal;
    WorldPos0 = (gWorld * Pos4).xyz;
    LightSpacePos = gLightWVP * Pos4;
}
