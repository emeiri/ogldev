//
#version 460

layout (location=0) out vec2 uv;

// https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
void main() {
  uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);

  gl_Position = vec4(uv * 2.0 + -1.0, 0.0, 1.0);
}
