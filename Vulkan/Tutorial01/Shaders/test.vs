#version 460

layout(binding = 0) uniform UniformBuffer {
    mat4 WVP;
} ubo;

vec2 pos[3] = vec2[3]( vec2(-0.7, 0.7), vec2(0.7, 0.7), vec2(0.0, -0.7) ); 

vec2 texcoords[3] = vec2[](
	vec2(1.0f, 0.0f),
	vec2(0.0f, 0.0f),
	vec2(0.0f, 1.0f)
);

layout(location = 0) out vec2 texCoord;


void main() 
{
    gl_Position = ubo.WVP * vec4( pos[gl_VertexIndex], 0.0, 1.0 );
	
	texCoord  = texcoords[gl_VertexIndex];
	//gl_Position = vec4( pos[gl_VertexIndex], 0.0, 1.0 );
}

