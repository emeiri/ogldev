#version 330

in vec3 WorldPos;

uniform vec3 gLightWorldPos;

out float LightToPixelDistance;

void main()
{
    vec3 LightToVertex = WorldPos - gLightWorldPos;

    LightToPixelDistance = length(LightToVertex);
}
