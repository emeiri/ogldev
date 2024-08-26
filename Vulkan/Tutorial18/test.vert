#version 460

layout (binding = 0) uniform UniformBuffer {
    mat4 WVP;
} ubo;

layout(location = 0) out vec2 texCoord;

struct VertexData
{
	float x, y, z;
	float u, v;
};

layout (binding = 1) readonly buffer Vertices { VertexData data[]; } in_Vertices;
layout (binding = 2) readonly buffer Indices { uint data[]; } in_Indices;

void main() 
{
	uint Index = in_Indices.data[gl_VertexIndex];
	VertexData vtx = in_Vertices.data[Index];

	vec3 pos = vec3(vtx.x, vtx.y, vtx.z);

	gl_Position = ubo.WVP * vec4(pos, 1.0);

	texCoord = vec2(vtx.u, vtx.v);
}

