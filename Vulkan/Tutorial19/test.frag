#version 460

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 out_Color;

layout(binding = 3) uniform sampler2D texSampler;

void main() {
//  out_Color = vec4( 0.0, 0.4, 1.0, 1.0 ); 
  out_Color = texture(texSampler, uv);
}
