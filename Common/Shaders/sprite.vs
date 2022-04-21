#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in uint PrimID;

#define MAX_QUADS 100

struct Quad {
       vec2 BasePos;
       vec2 WidthHeight;
};

uniform Quad gQuads[MAX_QUADS];

void main()
{
    vec3 BasePos = vec3(gQuads[PrimID].BasePos, 0.0);
    vec3 WidthHeight =  Position * vec3(gQuads[PrimID].WidthHeight, 1.0);

    vec3 NewPosition = BasePos + WidthHeight;

    gl_Position = vec4(NewPosition, 1.0);
}
