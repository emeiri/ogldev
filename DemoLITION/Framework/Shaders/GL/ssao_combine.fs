/**/
#version 460 core

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texScene;
layout(binding = 1) uniform sampler2D texSSAO;

layout(std140, binding = 0) uniform SSAOParams
{
	float scale;
	float bias;
};

void main()
{
	vec4 color = texture(texScene, TexCoords);
	float ssao = clamp( texture(texSSAO, TexCoords).r + bias, 0.0, 1.0 );

	outColor = vec4(mix(color, color * ssao, scale).rgb, 1.0);
}
