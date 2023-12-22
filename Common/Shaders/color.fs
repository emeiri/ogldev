#version 400

uniform vec4 gColor = vec4(1.0);

layout ( location = 0 ) out vec4 FragColor;

void main()
{
    FragColor = gColor;
}
