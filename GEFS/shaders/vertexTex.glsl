#version 150 core

in vec3 position;

/* 
const int numLights = 3;
vec3 inLightDir[numLights] = vec3[numLights](
     normalize(vec3(-1,-1,-1)),
     normalize(vec3(-.3,.3,-1)),
     normalize(vec3(1,1,-1)));
uniform vec3 inLightCol[numLights] = vec3[numLights](
     1*vec3(1,1,1),
     .5*vec3(1,1,1),
     .6*vec3(1,1,1));
     
/*/

const int maxNumLights = 5;
uniform int numLights;
//x,-z,y - environment map order
uniform vec3 inLightDir[maxNumLights];
uniform vec3 inLightCol[maxNumLights];
//*/
in vec3 inNormal;
in vec2 inTexcoord;

out vec3 Color;
out vec3 interpolatedNormal;
out vec3 pos;
out vec3 lightDir[maxNumLights];
out vec3 lightCol[maxNumLights];
out vec2 texcoord;
out vec4 shadowCoord;
out vec3 globalPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 shadowView;
uniform mat4 shadowProj;

void main() {
  gl_Position = proj * view * model * vec4(position,1.0);
  pos = (view * model * vec4(position,1.0)).xyz;
  globalPos = (model * vec4(position,1.0)).xyz;
  for (int i = 0; i < numLights; i++){
    lightDir[i] = (view * vec4(inLightDir[i],0.0)).xyz; //It's a vector!
    lightCol[i] = inLightCol[i];
  }
  vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);
  interpolatedNormal = normalize(norm4.xyz);
  texcoord = inTexcoord;

  shadowCoord = shadowProj * shadowView * model * vec4(position,1);
}