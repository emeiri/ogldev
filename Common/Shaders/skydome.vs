#version 410

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoords;

uniform mat4 gWVP = mat4(1.0);

out vec2 TexCoords0;
out float Height;
out vec3 StarPos;

void main()
{
    gl_Position = gWVP * vec4(Position, 1.0);
    gl_Position.z = gl_Position.w;
    Height = gl_Position.y / gl_Position.w;
    TexCoords0 = TexCoords;
    StarPos = normalize(Position);
}
