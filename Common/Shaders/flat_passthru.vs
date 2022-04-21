#version 330

layout (location = 0) in vec3 Position;

struct Quad {
       vec2 BasePos;
       vec2 WidthHeight;
};

uniform Quad gQuads[1];


void main()
{
    vec3 NewPosition = vec3(gQuads[0].BasePos, 0.0) + Position * vec3(gQuads[0].WidthHeight, 1.0);

    gl_Position = vec4(NewPosition, 1.0);
}
