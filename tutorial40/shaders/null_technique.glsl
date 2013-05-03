uniform mat4 gWVP;

shader VSmain(in vec3 Position)
{
    gl_Position = gWVP * vec4(Position, 1.0);
};

shader FSmain()
{
};

program NullTechnique
{
    vs(410) = VSmain();
    fs(410) = FSmain();
};