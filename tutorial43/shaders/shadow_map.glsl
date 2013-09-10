uniform mat4 gWVP;
                  
shader VSmain(in vec3 Position, in vec2 TexCoord, in vec3 Normal, out vec2 TexCoordOut)
{
    gl_Position = gWVP * vec4(Position, 1.0);
    TexCoordOut = TexCoord;
}

in vec2 GSTexCoordIn[3];

shader GSmain(out vec2 TexCoord)
{
    gl_Position = gl_in[0].gl_Position;
    TexCoord = GSTexCoordIn[0];
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    TexCoord = GSTexCoordIn[1];
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    TexCoord = GSTexCoordIn[2];
    EmitVertex();

    EndPrimitive();
}

uniform sampler2D gShadowMap;

shader FSmain(in vec2 TexCoordOut, out vec4 FragColor)
{
    float Depth = texture(gShadowMap, TexCoordOut).x;
    FragColor = vec4(Depth);
}


program ShadowMap
{
    vs(330)=VSmain();
    gs(330)=GSmain() : in(triangles) , out(triangle_strip,max_vertices = 3);
    fs(330)=FSmain();
};