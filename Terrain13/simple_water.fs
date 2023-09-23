#version 330

layout(location = 0) out vec4 FragColor;

in vec2 oTex;
in float oHeight;
in float oMaxHeight;

void main()
{
    float Color = (oHeight + oMaxHeight) / (2 * oMaxHeight);

    FragColor = vec4(15.0/255.0, 44.0/255.0, Color, 1);
}
