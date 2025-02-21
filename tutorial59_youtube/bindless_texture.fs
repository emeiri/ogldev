#version 460 core

#extension GL_ARB_bindless_texture : require

uniform int gTextureIndex = 0;

layout(binding = 0, std430) readonly buffer ssbo3 {
    sampler2D textures[];
};

smooth in vec2 TexCoord;
flat in int Instance;

out vec4 FragColor;

void main() 
{   
    sampler2D tex = textures[gTextureIndex];
    
    FragColor = vec4(texture(tex, TexCoord).rgb, 1.0);
  //FragColor = vec4(float(Instance) / 350.0);
}