#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

uniform mat4 gWVP;
uniform float gHeight = 0.0f;

out vec2 oTex;

void main()
{
    vec3 NewPosition = (Position + vec3(0.0, gHeight, 0.0));
    gl_Position = gWVP * vec4(NewPosition, 1.0);
    oTex = TexCoord;
}
