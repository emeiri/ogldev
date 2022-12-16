#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Color;

uniform mat4 gVP;

out vec4 oColor;

void main()
{
    gl_Position = gVP * vec4(Position, 1.0);

    oColor = vec4(Color, 1.0);
}
