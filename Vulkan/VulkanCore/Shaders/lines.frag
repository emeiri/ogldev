#version 460

layout(location = 0) in vec4 vs_Color;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vs_Color;
};
