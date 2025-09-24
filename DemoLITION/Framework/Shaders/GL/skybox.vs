// Copied from "The 3D graphics Rendering cookbook 2nd edition":
// https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook-Second-Edition/blob/main/Chapter04/04_CubeMap/src/skybox.vert

#version 460 core

layout (location=0) out vec3 dir;

uniform mat4 gVP = mat4(1.0);

const vec3 pos[8] = vec3[8](
	vec3(-1.0,-1.0, 1.0),		// 0
	vec3( 1.0,-1.0, 1.0),		// 1
	vec3( 1.0, 1.0, 1.0),		// 2
	vec3(-1.0, 1.0, 1.0),		// 3

	vec3(-1.0,-1.0,-1.0),		// 4
	vec3( 1.0,-1.0,-1.0),		// 5
	vec3( 1.0, 1.0,-1.0),		// 6
	vec3(-1.0, 1.0,-1.0)		// 7
);

const int indices[36] = int[36](
	0, 1, 2, 2, 3, 0,	// front
	1, 5, 6, 6, 2, 1,	// right 
	7, 6, 5, 5, 4, 7,	// back
	4, 0, 3, 3, 7, 4,	// left
	4, 5, 1, 1, 0, 4,	// bottom
	3, 2, 6, 6, 7, 3	// top
);

void main() 
{
	int idx = indices[gl_VertexID];
	vec4 Pos = vec4(pos[idx], 1.0);
	vec4 WVP_Pos = gVP * Pos;
	gl_Position = WVP_Pos.xyww;
	dir = pos[idx].xyz;
}
