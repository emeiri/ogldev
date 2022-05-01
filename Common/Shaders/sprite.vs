#version 330

layout (location = 0) in vec2 Position;
layout (location = 1) in uint QuadID;

#define MAX_QUADS 100

uniform QuadInfo {
       vec2 BasePos[MAX_QUADS];
       vec2 WidthHeight[MAX_QUADS];
       vec2 TexCoords[MAX_QUADS];
       vec2 TexWidthHeight[MAX_QUADS];
};

out vec2 TexCoords0;

void main()
{
    // Calculate position
    vec3 BasePos = vec3(BasePos[QuadID], 0.5);

    vec2 WidthHeight = Position * WidthHeight[QuadID];

    vec3 NewPosition = BasePos + vec3(WidthHeight, 0.0);

    gl_Position = vec4(NewPosition, 1.0);

    // Calculate tex coords
    vec2 BaseTexCoords = TexCoords[QuadID];

    vec2 TexWidthHeight = Position * TexWidthHeight[QuadID];

    TexCoords0 = BaseTexCoords + TexWidthHeight;
}
