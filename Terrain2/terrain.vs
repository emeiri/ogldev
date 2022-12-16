#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 InColor;

uniform mat4 gVP;

out vec4 Color;

void main()
{
    gl_Position = gVP * vec4(Position, 1.0);

    Color = vec4(InColor, 1.0);
}
