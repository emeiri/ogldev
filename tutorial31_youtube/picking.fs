#version 330

uniform uint gObjectIndex;
uniform uint gDrawIndex;

out uvec3 FragColor;

void main()
{
   FragColor = uvec3(gObjectIndex, gDrawIndex, gl_PrimitiveID);
}