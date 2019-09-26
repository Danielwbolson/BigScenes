#version 150 core

in vec3 Color;
in vec2 Texcoord;

out vec4 outColor;

uniform bool horizontalBlur;
const int blurSize = 11;
uniform float weights[5] = float[] (.227, .195, .122, .054, .016); 

uniform sampler2D image;

void main() {
  float denom = (blurSize+1)*(blurSize+1);
  float weight = 1/denom; //1/121; //1.0/(2*blurSize+1);
  vec2 pixelSize =  1.0 / textureSize(image, 0); // gets size of single texel
  vec3 result = texture(image, Texcoord).rgb * weight;//s[0];
  if(horizontalBlur){
    for(int i = 1; i < blurSize; ++i){
      weight = (blurSize-i)/denom; //(10-i)/121.f;
      result += texture(image, Texcoord + vec2(pixelSize.x * i, 0.0)).rgb * weight;//s[i];
      result += texture(image, Texcoord - vec2(pixelSize.x * i, 0.0)).rgb * weight;//s[i];
    }
  }
  else { //vetical blur
    for(int i = 1; i < blurSize; ++i){
        weight = (blurSize-i)/denom; //(10-i)/121.f;
        result += texture(image, Texcoord + vec2(0.0, pixelSize.y * i)).rgb * weight;//s[i];
        result += texture(image, Texcoord - vec2(0.0, pixelSize.y * i)).rgb * weight;//s[i];
    }
  }

  outColor = vec4(result,1);
  //outColor.b = 0;
}