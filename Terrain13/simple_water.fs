#version 330

layout(location = 0) out vec4 FragColor;

in vec2 oTex;

void main()
{
    FragColor = vec4(15.0/255.0, 94.0/255.0, 156.0/255.0, 1);
}
