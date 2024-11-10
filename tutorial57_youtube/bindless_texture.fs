#version 460 core

#extension GL_ARB_bindless_texture : require

// SSBO containing the textures
layout(binding = 0, std430) readonly buffer ssbo3 {
    sampler2D textures[];
};

smooth in vec2 TexCoord;

out vec4 FragColor;

void main() 
{   
    sampler2D tex = textures[0];
    
    FragColor = vec4(texture(tex, TexCoord).rgb, 1.0);
}