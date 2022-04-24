#version 440

layout (location = 0) in vec2 Position;
layout (location = 1) in uint PrimID;

#define MAX_QUADS 100

layout (binding = 0) uniform QuadInfo {
       vec2 BasePos[MAX_QUADS];
       vec2 WidthHeight[MAX_QUADS];
       vec2 TexCoords[MAX_QUADS];
       vec2 TexWidthHeight[MAX_QUADS];
};

out vec2 TexCoords0;

void main()
{
    // Calculate position
    vec3 BasePos = vec3(BasePos[PrimID], 0.0);

    vec3 WidthHeight =  vec3(Position, 0.5f) * vec3(WidthHeight[PrimID], 1.0);

    vec3 NewPosition = BasePos + WidthHeight;

    gl_Position = vec4(NewPosition, 1.0);

    // Calculate tex coords
    vec2 BaseTexCoords = TexCoords[PrimID];

    vec2 TexWidthHeight = Position * TexWidthHeight[PrimID];

    TexCoords0 = BaseTexCoords + TexWidthHeight;
}
