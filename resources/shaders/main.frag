#version 460

layout(location=0) in vec3 color;
layout(location=0) out vec4 outFragColor;
//layout(push_constant) uniform PushConstants{
//  float value;
//}pc;

void main(){
  outFragColor = vec4(color , 1.0);
}
