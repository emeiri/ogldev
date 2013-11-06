uniform mat4 gWVP;
uniform mat4 gWorld;
uniform vec3 gLightWorldPos;
                  
shader VSmain(in vec3 Position, in vec2 TexCoord, in vec3 Normal, out vec3 LightToVertex)
{
    vec4 Pos4 = vec4(Position, 1.0);
    gl_Position = gWVP * Pos4;
    vec4 WorldPos = gWorld * Pos4;
    LightToVertex = gLightWorldPos - WorldPos.xyz;
}

shader FSmain(in vec3 LightToVertex,out float FragColor)
{
    float LightToPixelDistance = length(LightToVertex);

    FragColor = LightToPixelDistance;
}


program ShadowMap
{
    vs(330)=VSmain();
    fs(330)=FSmain();
};