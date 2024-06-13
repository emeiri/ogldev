#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 Bitangent;

uniform mat4 gWVP;
uniform mat4 gLightWVP;
uniform mat4 gWorld;
uniform mat3 gNormalMatrix;
uniform mat4 gWV;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 WorldPos0;
out vec4 LightSpacePos0;
out vec3 Tangent0;
out vec3 Bitangent0;

void main()
{
    vec4 Pos4 = vec4(Position, 1.0);
    gl_Position = gWVP * Pos4;
    TexCoord0 = TexCoord;
    Normal0 = gNormalMatrix * Normal;
    Tangent0 = gNormalMatrix * Tangent;
    Bitangent0 = gNormalMatrix * Bitangent;
    WorldPos0 = (gWorld * Pos4).xyz;
    LightSpacePos0 = gLightWVP * Pos4;
}
