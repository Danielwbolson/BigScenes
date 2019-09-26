#ifndef SCENE_H
#define SCENE_H

#include "glm/glm.hpp"

#include <vector>
#include <string>


struct Light{
	glm::vec3 color = glm::vec3(1,1,1);
	glm::vec3 direction = glm::vec3(-0.4 -0.1 -1.0);
	float distance = 10;
	float intensity = 3;
	bool castShadow = false; //TODO: Only 1 light can cast a shadow
	float frustLeft = -5.0f, frustRight = 5.0f;
	float frustBot = -5.0f, frustTop = 5.0f;
	float frustNear = 0.6f, frustFar = 20.0f;
	float shadowBias = 0.001;
	int pcfWidth = 1;
	std::string name = "**UNNAMED LIGHT***";
};
//TODO: Add point lights (we currently assume all lights are directional)

struct Camera{
	float FOV = 50;
};

struct Model;

struct Scene{
	std::string environmentMap = "";
	Light shadowLight;
	std::vector<Light> lights;
	Camera mainCam;
	glm::mat4 rotSkybox; //skybox orientation
	bool singleSkyColor = true;
	glm::vec3 skyColor = glm::vec3(1,1,10);
	glm::vec3 ambientLight = glm::vec3(0.3, 0.3, 0.3);

	std::vector<Model*> toDraw;
};

void loadScene(std::string fileName);
void resetScene();

//TODO: Allow user to control lights via Lua script
//TODO: Allow the user to parent models to lights


//Global scene and list of lights/color
extern Scene curScene;
extern glm::vec3 lightDirections[20];
extern glm::vec3 lightColors[20];

#endif //SCENE_H