#version 400

layout (location = 0) in vec3 Position;

void main()
{
    gl_Position = vec4(Position.x, Position.y, Position.z, 1.0);
}