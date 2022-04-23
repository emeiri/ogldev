#version 410

layout (location = 0) in vec2 Position;
layout (location = 1) in uint PrimID;

#define MAX_QUADS 100

struct Quad {
       vec2 BasePos;
       vec2 WidthHeight;
       vec2 TexCoords;
       vec2 TexWidthHeight;
};

uniform Quad gQuads[MAX_QUADS];

out vec2 TexCoords0;


void main()
{
    // Calculate position
    vec3 BasePos = vec3(gQuads[PrimID].BasePos, 0.0);

    vec3 WidthHeight =  vec3(Position, 0.5f) * vec3(gQuads[PrimID].WidthHeight, 1.0);

    vec3 NewPosition = BasePos + WidthHeight;

    gl_Position = vec4(NewPosition, 1.0);

    // Calculate tex coords
    vec2 BaseTexCoords = gQuads[PrimID].TexCoords;

    vec2 TexWidthHeight = Position * gQuads[PrimID].TexWidthHeight;

    TexCoords0 = BaseTexCoords + TexWidthHeight;
}
