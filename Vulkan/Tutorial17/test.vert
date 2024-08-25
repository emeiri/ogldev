#version 460

layout (binding = 0) uniform UniformBuffer {
    mat4 WVP;
} ubo;

struct VertexData
{
	float x, y, z;
	float u, v;
};

layout (binding = 1) readonly buffer Vertices { VertexData data[]; } in_Vertices;

void main() 
{
	VertexData vtx = in_Vertices.data[gl_VertexIndex];

	vec3 pos = vec3(vtx.x, vtx.y, vtx.z);

	gl_Position = ubo.WVP * vec4(pos, 1.0);
}

