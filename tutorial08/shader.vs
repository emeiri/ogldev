#version 330

layout (location = 0) in vec3 Position;

uniform mat4 gScaling;

void main()
{
    gl_Position = gScaling * vec4(Position, 1.0);
}
