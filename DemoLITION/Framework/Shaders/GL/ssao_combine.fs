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

// composite pass
uniform int gDebugMode = 1; // 0 = normal, 1 = SSGI only
uniform float gSSGIIntensity = 0.5;

void main()
{
    vec3 direct   = texture(texScene, TexCoords).rgb;
    vec3 indirect = texture(texSSAO, TexCoords).rgb * gSSGIIntensity;

    vec3 color = (gDebugMode == 1) ? indirect : (direct * 0.5 + indirect);
    FragColor = vec4(color, 1.0);
  //  FragColor = vec4(direct, 1.0);
}
