uniform mat4 gWVP;
                  
shader VSmain(in vec3 Position, in vec2 TexCoord, in vec3 Normal)
{
    gl_Position = gWVP * vec4(Position, 1.0);
}


shader FSmain()
{
}


program ShadowMap
{
    vs(330)=VSmain();
    fs(330)=FSmain();
};