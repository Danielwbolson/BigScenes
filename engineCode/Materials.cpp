#include "Materials.h"

#include <external/loguru.hpp>

#include <string>
using std::string;

Material materials[1000];
int numMaterials = 0;

string textures[1000];
int numTextures = 0;

void resetMaterials(){
  numMaterials = 0;
  numTextures = 0;
}


void loadMaterials(string fileName){
  LOG_SCOPE_FUNCTION(INFO);

  FILE *fp;
  long length;
  char rawline[1024]; //Assumes no line is longer than 1024 characters!

  // open the file containing the material descriptions
  fp = fopen(fileName.c_str(), "r");
  
  CHECK_NOTNULL_F(fp,"Can't open material file '%s'", fileName.c_str());
	
  fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
  length = ftell(fp);  // return the value of the current position
  LOG_F(2,"File '%s' is %ld bytes long.",fileName.c_str(),length);
  fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
  
  string textureDir = "./";
	int curMaterialID;
  //Loop through reading each line
  while( fgets(rawline,1024,fp) ) { //Assumes no line is longer than 1024 characters!
	  string line = string(rawline);
    if (rawline[0] == '#'){
      LOG_F(2,"Skipping comment: %s", rawline);
      continue;
    }
    
    char command[100];
    int fieldsRead = sscanf(rawline,"%s ",command); //Read first word in the line (i.e., the command type)
    string commandStr = command;
    
    if (fieldsRead < 1){ //No command read = Blank line
     continue;
    }
    
		string matName = "";
    if (commandStr.substr(0,1) == "["){ // "[" indicates new material
		  int closeBrackets = line.find("]");
      CHECK_F(closeBrackets >= 0,"ERROR: Material name opened with [ but not closed with ]");

      matName = line.substr(1,closeBrackets-1);
			curMaterialID = numMaterials;
			numMaterials++;
      materials[curMaterialID] = Material(); //Make new default material
			materials[curMaterialID].name = matName;
			LOG_F(1,"Creating material with name: %s",matName.c_str());
    }
    else if (commandStr == "metallic"){ 
			float metallicValue;
			sscanf(rawline,"metallic = %f", &metallicValue);
			materials[curMaterialID].metallic = metallicValue;
      LOG_F(1,"Metalic property of %f",metallicValue);
    }
		else if (commandStr == "smoothness"){
			float roughnessValue;
			sscanf(rawline,"smoothness = %f", &roughnessValue);
      float v = 1-roughnessValue;
      v = fmaxf(v,0);
      float smoothnessValue = 1 - roughnessValue*roughnessValue;; //roughnessValue;  //sqrt(v);// 1 - roughnessValue*roughnessValue;
      smoothnessValue = std::fmaxf(smoothnessValue,0.01);
			materials[curMaterialID].roughness = smoothnessValue;
      LOG_F(1,"Roughness property of %f",smoothnessValue);
    }
		else if (commandStr == "ior"){ 
			float iorValue;
			sscanf(rawline,"ior = %f", &iorValue);
			materials[curMaterialID].ior = iorValue;
      LOG_F(1,"ior property of %f",iorValue);
    }else if (commandStr == "reflectiveness"){ 
			float reflectiveness;
			sscanf(rawline,"reflectiveness = %f", &reflectiveness);
			materials[curMaterialID].reflectiveness = reflectiveness;
      LOG_F(1,"reflectiveness of %f",reflectiveness);
    }else if (commandStr == "emissive"){ 
			float colR, colG, colB;
			sscanf(rawline,"emissive = %f %f %f", &colR, &colG, &colB);
			materials[curMaterialID].emissive = glm::vec3(colR, colG, colB);
      LOG_F(1,"Emissve color of: %f, %f, %f",colR,colG,colB);
    }
		else if (commandStr == "color"){ 
			float colR, colG, colB;
			sscanf(rawline,"color = %f %f %f", &colR, &colG, &colB);
			materials[curMaterialID].col = glm::vec3(colR,colG,colB);
      LOG_F(1,"Color of: %f, %f, %f",colR,colG,colB);
    }
    else if (commandStr == "textureDir"){ 
       char dirName[1024];
       sscanf(rawline,"textureDir = %s", dirName);
			 textureDir = dirName;
			 LOG_F(1,"Setting texture directory to: %s",textureDir.c_str());
		}
    else if (commandStr == "texture"){ 
       char rawTextureName[1024];
       sscanf(rawline,"texture = %s", rawTextureName);
       string textureName = textureDir + string(rawTextureName);
			 int foundTexture = -1;
			 for (int i = 0; i < numTextures; i++){
				 if (textures[i] == textureName){
					 foundTexture = i;
					 break;
				 }
			 }
			 if (foundTexture >= 0){
				 LOG_F(1,"Reusing existing texture: %s", textures[foundTexture].c_str());
				 materials[curMaterialID].textureID = foundTexture;
			 }
			 else{
			   textures[numTextures] = textureName;
		     materials[curMaterialID].textureID = numTextures;
			   numTextures++;
				 LOG_F(1,"New texture named: %s", textureName.c_str());
			 }
    }
    else {
      LOG_F(WARNING,"WARNING. Unknow command: %s in file %s",command,fileName.c_str());
    }
  }
}

int findMaterial(string materialName){
	for (int i = 0; i < numMaterials; i++){
		if (materials[i].name == materialName){
			return i;  //MaterialID
		} 
	}
	LOG_F(ERROR,"No material model of name '%s' found!",materialName.c_str());
	exit(1);
}