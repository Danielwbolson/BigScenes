#version 330 core

in vec2 texcoord;
in vec2 position;

out vec2 Texcoord;

void main() {
   gl_Position = vec4(position,0,1.0);
   Texcoord = texcoord;
}