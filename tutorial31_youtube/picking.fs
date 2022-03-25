#version 330

uniform uint gDrawIndex;
uniform uint gObjectIndex;

out uvec3 FragColor;

void main()
{
   FragColor = uvec3(gObjectIndex, gDrawIndex, gl_PrimitiveID + 1);
}