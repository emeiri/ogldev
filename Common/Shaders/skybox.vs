#version 330

layout (location = 0) in vec3 Position;

uniform mat4 gWVP;

out vec3 TexCoord0;

void main()
{    
    gl_Position = gWVP * vec4(Position, 1.0);
    TexCoord0   = Position;
}
