#version 460

layout (location = 0) out vec3 color;
//layout(push_constant) uniform PushConstants{
//  float value;
//}pc;

const vec2 pos[3] = vec2[3](
    vec2(-0.6, -0.4), vec2(0.6, -0.4), vec2(0.0, 0.0));
const vec3 col[3] = vec3[3](
      vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));


void main() {
  gl_Position = vec4(pos[gl_VertexIndex],0.5, 1.0);
  color = col[gl_VertexIndex];
}
