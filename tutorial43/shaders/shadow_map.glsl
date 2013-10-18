uniform mat4 gWVP;
uniform mat4 gWV;
                  
shader VSmain(in vec3 Position, in vec2 TexCoord, in vec3 Normal, out vec3 ViewPos)
{
    vec4 Pos4 = vec4(Position, 1.0);
    gl_Position = gWVP * Pos4;
    ViewPos = (gWV * Pos4).xyz;
}

out float FragColor;

shader FSmain(in vec3 ViewPos)
{
    float LightToPixelDistance = length(ViewPos);

    FragColor = LightToPixelDistance;
}


program ShadowMap
{
    vs(330)=VSmain();
    fs(330)=FSmain();
};