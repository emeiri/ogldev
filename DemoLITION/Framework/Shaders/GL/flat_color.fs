#version 330

layout(location = 0) out vec4 FragColor;

uniform vec4 gColor;

void main()
{
    FragColor = gColor;
}
