#ifndef SHADER_H
#define SHADER_H

#include <string>

using std::string;

struct Shader{
  std::string VertexShaderName;
  std::string FragmentShaderName;
  int ID = -1; //shader ID for OpenGL

  Shader(std::string VertShader, std::string FragShader) : VertexShaderName(VertShader), FragmentShaderName(FragShader){};

  Shader(){};

  void init();  //Load, Compile, and Link shaders

  void bind();

  ~Shader();
};

#endif //SHADER_H