#version 460

layout(location = 0) out vec4 vs_Color;

layout(binding=0) uniform UBO {
	mat4 WVP;
	float time;
} ubo;

struct VertexData
{
	float x, y, z;
	float r, g, b, a;
};

layout(binding=1) readonly buffer SBO { VertexData data[]; } sbo;

void main()
{
	VertexData v = sbo.data[gl_VertexIndex];

	gl_Position = ubo.WVP * vec4(v.x, v.y, v.z, 1.0);
	vs_Color = vec4(v.r, v.g, v.b, v.a);
}
