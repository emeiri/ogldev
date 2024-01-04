#version 400

layout (location = 0) in vec2 Position;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(Position.x, Position.y, 0.0, 1.0);

    TexCoord = (Position + vec2(1.0)) / 2.0;
}