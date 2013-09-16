uniform mat4 gWVP[6];
                  
shader VSmain(in vec3 Position)
{
    gl_Position = vec4(Position, 1.0);
}

shader GSmain(out vec2 TexCoord)
{
    for (int layer = 0 ; layer < 6 ; layer++) {
        for (int vertex = 0 ; vertex < 3 ; vertex++) {
            gl_Layer = layer;
            gl_Position = gWVP[layer] * gl_in[vertex].gl_Position;
            EmitVertex();            
        }

        EndPrimitive();
    }
}

shader FSmain(out vec4 FragColor)
{
    FragColor = vec4(1.0f);
}


program ShadowMap
{
    vs(330)=VSmain();
    gs(330)=GSmain() : in(triangles) , out(triangle_strip,max_vertices = 18);
    fs(330)=FSmain();
};