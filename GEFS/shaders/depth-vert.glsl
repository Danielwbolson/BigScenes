#version 330 core

in vec3 position; //Changes for each vertex

out vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;


void main() {
  gl_Position = proj * view * model * vec4(position,1.0);
  pos = (view * model * vec4(position,1.0)).xyz;
}