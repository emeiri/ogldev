uniform mat4 gWVP;
uniform mat4 gWV;
                  
shader VSmain(in vec3 Position, in vec2 TexCoord, in vec3 Normal, out vec3 ViewPos)
{
    vec4 Pos4 = vec4(Position, 1.0);
    gl_Position = gWVP * Pos4;
    ViewPos = (gWV * Pos4).xyz;
}

out vec4 FragColor;

shader FSmain(in vec3 ViewPos)
{
    FragColor = vec4(length(ViewPos));
}


program ShadowMap
{
    vs(330)=VSmain();
    fs(330)=FSmain();
};