#version 330

layout (location = 0) in vec3 Position;

uniform mat4 gVP;

out vec4 Color;

void main()
{
    gl_Position = gVP * vec4(Position, 1.0);

    Color = vec4(Position.y / 200.0);
}
