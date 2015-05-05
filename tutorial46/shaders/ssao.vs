#version 330                                                                        

layout (location = 0) in vec3 Position; 

uniform float gAspectRatio;
uniform float gTanHalfFOV;

out vec2 TexCoord;
out vec2 ViewRay;

void main()
{          
    gl_Position = vec4(Position, 1.0);
    TexCoord = (Position.xy + vec2(1.0)) / 2.0;
    ViewRay.x = Position.x * gAspectRatio * gTanHalfFOV;
    ViewRay.y = Position.y * gTanHalfFOV;
}
