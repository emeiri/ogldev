uniform mat4 gWVP;
                  
shader VSmain(in vec3 Position, in vec2 TexCoord, in vec3 Normal, out vec2 TexCoordOut)
{
    gl_Position = gWVP * vec4(Position, 1.0);
    TexCoordOut = TexCoord;
}

uniform sampler2D gShadowMap;

shader FSmain(in vec2 TexCoordOut, out vec4 FragColor)
{
    float Depth = texture(gShadowMap, TexCoordOut).x;
//    Depth = 1.0 - (1.0 - Depth) * 25.0;
    FragColor = vec4(Depth);
}


program ShadowMap
{
    vs(330)=VSmain();
    fs(330)=FSmain();
};