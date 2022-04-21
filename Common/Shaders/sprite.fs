#version 330

layout(location = 0) out vec4 FragColor;

uniform vec3 gColor;

void main()
{
    FragColor = vec4(gColor, 1.0);
}
