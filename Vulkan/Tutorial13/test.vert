#version 460

vec2 pos[3] = vec2[3]( vec2(-0.7, 0.7), vec2(0.7, 0.7), vec2(0.0, -0.7) ); 

void main() 
{
	gl_Position = vec4( pos[gl_VertexIndex], 0.0, 1.0 );
}

