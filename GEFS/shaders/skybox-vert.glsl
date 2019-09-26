#version 330 core
layout (location = 0) in vec3 pos;

out vec3 texCoords;

uniform mat4 proj;
uniform mat4 view;

uniform mat4 rotSkybox;

void main(){
    vec3 inPos = pos; //vec3(pos.x, pos.y, pos.z);
    //texCoords = inPos; //3D position is used as lookup into texture coordinate
    //vec4 newPos = view * vec4(pos,0); //Right up, but wrong rest of it
    texCoords = (rotSkybox*vec4(pos,1)).xyz;
    vec4 outPos = proj * view * vec4(inPos, 1.0);
    gl_Position = outPos.xyww; //Set depth to be 1 (furtherest possible)

}  