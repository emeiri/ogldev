#version 330

in vec2 TexCoord0;

uniform sampler2D gColorTexture;
uniform sampler2D gMotionTexture;
    
out vec4 FragColor;
                                                                                        
void main()
{                                    
    vec2 MotionVector = texture(gMotionTexture, TexCoord0).xy / 2.0;

    vec4 Color = vec4(0.0);

    vec2 TexCoord = TexCoord0;
    
    Color += texture(gColorTexture, TexCoord) * 0.4;
    TexCoord -= MotionVector;
    Color += texture(gColorTexture, TexCoord) * 0.3;
    TexCoord -= MotionVector;
    Color += texture(gColorTexture, TexCoord) * 0.2;
    TexCoord -= MotionVector;
    Color += texture(gColorTexture, TexCoord) * 0.1;

    FragColor = Color;
}
