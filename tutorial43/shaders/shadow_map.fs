#version 330

in vec3 WorldPos;

uniform vec3 gLightWorldPos;
          
out float FragColor;
        
void main()
{
    vec3 LightToVertex = WorldPos - gLightWorldPos;

    float LightToPixelDistance = length(LightToVertex);

    FragColor = LightToPixelDistance;
}

