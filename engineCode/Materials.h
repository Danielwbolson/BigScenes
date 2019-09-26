#ifndef MATERIALS_H
#define MATERIALS_H

#include "glm/glm.hpp"

#include <string>

using std::string;

struct Material{
  std::string name = "**UNNAMED MATERIAL**";
	int textureID = -1; //no texture
	float ior = 1;
	float metallic = 0;
	float roughness = 0.5;
	float reflectiveness = 0;
	glm::vec3 emissive = glm::vec3(0,0,0);
	glm::vec3 col = glm::vec3(1,0,1); //bright default color
};

void loadMaterials(string fileName);

void resetMaterials();

void loadTexturesToGPU();

int findMaterial(string materialName);
                   

//Global list of materials
extern Material materials[1000];
extern int numMaterials;

//Global list of textures
extern string textures[1000];
extern int numTextures;

#endif