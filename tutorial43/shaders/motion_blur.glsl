struct VSInput 
{
    vec3  Position;                                             
    vec2  TexCoord;                                             
};

interface VSOutput
{                                                                                    
    vec2 TexCoord;                                                                 
};


shader VSmain(in VSInput VSin:0, out VSOutput VSout)
{       
    gl_Position    = vec4(VSin.Position, 1.0);
    VSout.TexCoord = VSin.TexCoord;
}

uniform sampler2D gColorTexture;
uniform sampler2D gMotionTexture;
                                                                                            
shader FSmain(in VSOutput FSin, out vec4 FragColor)
{                                    
    vec2 MotionVector = texture(gMotionTexture, FSin.TexCoord).xy / 2.0;

    vec4 Color = vec4(0.0);

    vec2 TexCoord = FSin.TexCoord;
    
    Color += texture(gColorTexture, TexCoord) * 0.4;
    TexCoord -= MotionVector;
    Color += texture(gColorTexture, TexCoord) * 0.3;
    TexCoord -= MotionVector;
    Color += texture(gColorTexture, TexCoord) * 0.2;
    TexCoord -= MotionVector;
    Color += texture(gColorTexture, TexCoord) * 0.1;

    FragColor = Color;
}

program MotionBlur
{
    vs(410)=VSmain();
    fs(410)=FSmain();
};