#version 460

vec2 pos[3] = vec2[3]( vec2(-0.7, 0.7), vec2(0.7, 0.7), vec2(0.0, -0.7) ); 

struct VertexData
{
	float x, y, z;
	float u, v;
};
layout(binding=1) readonly buffer Vertices { VertexData data[]; } in_Vertices;
void main() 
{
	VertexData vtx = in_Vertices.data[gl_VertexIndex];

	vec3 pos = vec3(vtx.x, vtx.y, vtx.z);

	gl_Position = vec4(pos, 1.0);
}

