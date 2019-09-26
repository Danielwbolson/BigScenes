#version 330 core
out vec4 outColor;

in vec3 texCoords;

uniform bool constColor;
uniform vec3 skyColor;

uniform samplerCube skybox;

void main(){
    if (constColor){
        outColor = vec4(skyColor,1);
    }
    else{
        vec4 envColor = texture(skybox, texCoords);
        outColor = 5*pow(envColor,vec4(5,5,5,1));
    }
    //outColor = vec4(texCoords,1);
}