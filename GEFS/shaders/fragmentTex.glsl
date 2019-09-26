#version 330 core

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 brightColor;

const int maxNumLights = 5; 

uniform int numLights;

in vec3 interpolatedNormal; //TODO: Just call this normal?
in vec3 pos;
in vec3 lightDir[maxNumLights];
in vec3 lightCol[maxNumLights];
//TODO: Support point lights

uniform vec3 modelColor;
uniform vec3 materialColor;
in vec2 texcoord;

uniform float metallic; //0-1  (When Metalic is 1, we ignore IOR)
uniform float roughness; //~.1-1
uniform float ior; //1.3-5? (stick ~1.3-1.5 unless gemstone or so)

uniform bool xxx;


uniform sampler2D colorTexture;
uniform vec2 textureScaleing;
uniform vec3 emissive;

uniform vec3 ambientLight;

uniform sampler2D shadowMap;
in vec4 shadowCoord;
uniform bool useShadow;
uniform int pcfSize;
uniform float shadowBias;

uniform int useTexture;

uniform vec3 skyColor;
uniform bool useSkyColor;
uniform mat4 invView; //inverse of view matrix
uniform samplerCube skybox;
uniform mat4 rotSkybox;
//uniform vec3 cameraPos;
uniform float reflectiveness;

//Cook-torrance basics with code
//http://filmicworlds.com/blog/optimizing-ggx-shaders-with-dotlh/
//http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx


//float G1V(float dotNV, float k){return 1.f/ (dotNV*(1-k)+k);}

float G1V(float dotNV, float k){return dotNV/ (dotNV*(1-k)+k);}  //Maybe better?

vec3 GGXSpec(vec3 N, vec3 V, vec3 L, float rough, vec3 F0){
  float alpha = rough*rough;

  vec3 H = normalize(V+L);

  float dotNL = clamp(dot(N,L),0.0,1.0);
  float dotNV = clamp(dot(N,V),0.0,1.0);
  float dotNH = clamp(dot(N,H),0.0,1.0);
  float dotLH = clamp(dot(L,H),0.0,1.0);
  float dotVH = clamp(dot(V,H),0.0,1.0);

  //D - Geometry distribution term (Microfaset distribution)
  //How focused the specular reflection is:
  // More roughness, less focused specular (in limit uniform)
  // Less roughness, more focused highlight 
  // Approximate idea: same amount of light gets reflected back, on smooth surfaces only a few
  // parts of the object will be pointing the right direction to reflect, but all nearby points reflect
  // strongly. On rough surfaces, a larger portion of the object will reflect back, but the reflection
  // is weaker (http://www.codinglabs.net/public/contents/article_physically_based_rendering_cook_torrance/images/ggx_distribution.jpg)
  float alphaSqr = alpha*alpha;
  float pi = 3.141592f;
  float denom = dotNH * dotNH * (alphaSqr - 1.0) + 1.0;
  float D = alphaSqr / (pi * denom * denom);

  //F - Fresnel
  //Response from light depends on angle (Schlick approximation)
  float dotLH5 = pow(1.0f - dotLH,5);
  vec3 F = F0 + (1.0 - F0)*(dotLH5);

  //G - Visibility term ... supposed to capture how microfacets shadow each other
  //Lights from smooth surfaces get brighter at grazing angles, but not rough surfaces (maybe ???)
  // This is due to micofacets shadowing each other
  float k = alpha*.5;
  float vis = G1V(dotNL,k)*G1V(dotNV,k); //Shlick-style approximation
  //vis = dotNL*dotNV; //Implicit ... fast to compute, but doesn't depend on roughness =/
  //vis = min(1.f,min(2*dotNH*dotNV/dotVH,2*dotNH*dotNL/dotVH)); //Original cook-torance formulation
  //Some hack I found somewhere online...
    //float k2 = k*k;
    //float invk2 = 1.0f-k2;
    //vis = 1/(dotLH*dotLH*invk2+k2); 

  if (dotNL <= 0) vis = 0;

  float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
  vec3 specular = vec3( D * F * vis);  //also a dotNL term here? I'm not sure
  return specular;
}

void main() {
  vec3 color;
  if (useTexture == 0)
    color = materialColor;
  else
    color = materialColor*texture(colorTexture, texcoord*textureScaleing).rgb;
  vec3 normal = normalize(interpolatedNormal);
  vec3 ambC = color*ambientLight;
  vec3 oColor = ambC+emissive;
   

  for (int i = 0; i < numLights; i++){
    float shadow = 0;
    float bias = shadowBias * max(2 * (1.0 - dot(normal, lightDir[i])),1); // const float bias = 0.01; //
    if (i == 0 && useShadow){ //TODO: Only the first light can have a shadow
      vec3 projCoords = vec3(shadowCoord.xyz/shadowCoord.w);
      projCoords = projCoords * 0.5 + 0.5; 
      float currentDepth = projCoords.z;
      //* //PCF:
      shadow = 0.0;
      vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
      for(int x = -pcfSize; x <= pcfSize; ++x){
        for(int y = -pcfSize; y <= pcfSize; ++y){
          float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
          shadow += currentDepth> pcfDepth + bias ? 0.95 : 0.0;   //Hack to keep some ambient in shadow regions
        }    
      }
      shadow /= (2*pcfSize+1)*(2*pcfSize+1);
      /*/ //Binary Shadow Map:
      float closestDepth = texture(shadowMap, projCoords.xy).r;
      shadow = currentDepth > closestDepth+bias ? 1.0 : 0.0;
      //*/
    }

    vec3 specC;
    vec3 lDir = lightDir[i];
    vec3 diffuseC = (1-metallic)*color*max(dot(-lDir,normal),0.0); //This is a Hack? Is it a good idea? Is it true metals have no diffuse color?
    vec3 viewDir = normalize(-pos); //We know the eye is at (0,0)!
    vec3 reflectDir = reflect(viewDir,normal);
    
    /*float spec = max(dot(reflectDir,lDir),0.0);
    if (dot(-lDir,normal) <= 0.0) spec = 0; //No specularity if light is behind object
    float m = 2*clamp(.6-roughness,0.0,1.0);
      specC = m*.8*vec3(1.0,1.0,1.0)*pow(spec,80*m/(ior*ior));
    if (m < .01) specC = vec3(0);*/

    vec3 iorVec = ior*vec3(1,1,1);

    vec3 F0 = abs ((1.0 - iorVec) / (1.0 + iorVec));
    F0 = F0 * F0;
    F0 = mix(F0,color,metallic);
    
    vec3 envColor = skyColor;
    if (!useSkyColor){
      vec3 eyeDir = normalize(pos.xyz); 
      vec3 RefVec = reflect(eyeDir.xyz, normal);
      RefVec = (rotSkybox*invView*vec4(RefVec,0)).xyz;
      envColor = texture(skybox, RefVec).rgb; 
      envColor = 5*pow(envColor,vec3(5,5,5)); //Hack to turn a non-HDR texture into an HDR one
    }    
    specC = lightCol[i]*GGXSpec(normal,viewDir,-lDir,roughness,F0);

    float ref = reflectiveness; //A simple reflectivness hack, really this should be part of sampling the BRDF
    specC += ref*envColor;

    oColor += (1-shadow)*(lightCol[i]*diffuseC+specC);
  }
  if (xxx) oColor = oColor.gbr; //Just to demonstrate how to use the boolean for debugging
  outColor = vec4(modelColor*oColor, 1.0);


  float brightness = dot(oColor, vec3(0.3, 0.6, 0.1));
  brightColor = outColor;
  if(brightness < 1)
      brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}