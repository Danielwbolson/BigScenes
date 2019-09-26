#version 150 core

in vec3 Color;
in vec2 Texcoord;

out vec4 outColor;

uniform float bloomAmount;

uniform sampler2D texDim;
uniform sampler2D texBright;


void main() {
   vec4 bright = texture(texBright, Texcoord);
   vec4 dim = texture(texDim, Texcoord);

   outColor = 1*dim + 1*bloomAmount*bright;

/*
   const float gamma = 2.2;
   const float exposure = 1.5;
  
    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-outColor.rgb * exposure);
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    outColor = vec4(mapped, 1.0);*/

    const float gamma = 2.2;//2.2;
   const float exposure = .2;

    //https://ninedegreesbelow.com/photography/xyz-rgb.html#XYZ
    //http://brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
    //http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html

    float X = .577*outColor.r + .186*outColor.g + .188*outColor.b;
    float Y = .297*outColor.r + .627*outColor.g + .075*outColor.b;
    float Z = .027*outColor.r + .071*outColor.g + .991*outColor.b;

    float x = X/(X+Y+Z);
    float y = Y/(X+Y+Z);

    Y = 1.0 - exp(-Y * exposure);

    float Xm = x*Y/y;
    float Ym = Y;
    float Zm = (1 - x - y)*Y/y;

    vec3 mapped;

    mapped.r = 2.041*Xm + -0.565*Ym - 0.345*Zm;
    mapped.g = -.963*Xm + 1.876*Ym + .04155*Zm;
    mapped.b = .0134*Xm + -0.118*Ym + 1.015*Zm;

    //if (xxx) mapped = oColor/(oColor+vec3(1)); //Reihard
    //if (xxx) mapped = vec3(1.0) - exp(-outColor.rgb * exposure); // Exposure tone mapping
    
    mapped = pow(mapped, vec3(1.0 / gamma)); // Gamma correction  

    outColor = vec4(mapped,1);

    //float depthValue = texture(texDim, Texcoord * 0.5 + 0.5).r;
    //outColor = vec4(vec3(depthValue), 1.0);
    
}