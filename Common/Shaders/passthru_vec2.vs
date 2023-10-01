#version 330

layout (location = 0) in vec2 Position;

void main()
{
    gl_Position = vec4(Position.x, Position.y, 0.0, 1.0);
}