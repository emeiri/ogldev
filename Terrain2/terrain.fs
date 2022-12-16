#version 330

layout(location = 0) out vec4 FragColor;

in vec4 oColor;


void main()
{
    FragColor = oColor;
}
