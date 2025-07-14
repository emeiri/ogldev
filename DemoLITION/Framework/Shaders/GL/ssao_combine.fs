/**/
#version 460 core

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D texScene;
layout(binding = 1) uniform sampler2D texSSAO;

layout(std140, binding = 0) uniform SSAOParams
{
	float Scale;
	float Bias;
};

void main()
{
	vec4 Color = texture(texScene, TexCoords);

	float SSAOFactor = texture(texSSAO, TexCoords).x;

	SSAOFactor = clamp(SSAOFactor + Bias, 0.0, 1.0);

	FragColor = vec4(mix(Color, Color * SSAOFactor, Scale).rgb, 1.0);
}
