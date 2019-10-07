#ifndef MODELS_H
#define MODELS_H

#include "Materials.h"
#include "CollisionSystem.h"

#include <vector>
#include <string>

struct Bounds {

	Bounds() : minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0), points(std::vector<glm::vec4>(8)) {}

	Bounds(const float minVx, const float minVy, const float minVz,
		   const float maxVx, const float maxVy, const float maxVz) : 
		   minX(minVx), minY(minVy), minZ(minVz),
		   maxX(maxVx), maxY(maxVy), maxZ(maxVz),
		   points(std::vector<glm::vec4>(8)) {
	
		points[0] = glm::vec4(maxX, maxY, maxZ, 1);
		points[1] = glm::vec4(maxX, minY, maxZ, 1);
		points[2] = glm::vec4(maxX, maxY, minZ, 1);
		points[3] = glm::vec4(maxX, minY, minZ, 1);
		points[4] = glm::vec4(minX, maxY, maxZ, 1);
		points[5] = glm::vec4(minX, minY, maxZ, 1);
		points[6] = glm::vec4(minX, maxY, minZ, 1);
		points[7] = glm::vec4(minX, minY, minZ, 1);

	}

	std::vector<glm::vec4> points;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	Bounds& operator = (const Bounds& b) {
		minX = b.minX;
		minY = b.minY;
		minZ = b.minZ;
		maxX = b.maxX;
		maxY = b.maxY;
		maxZ = b.maxZ;

		points = b.points;

		return *this;
	}
	glm::vec3 Max(const glm::mat4 t) {

		float newMaxX = -INFINITY;
		float newMaxY = -INFINITY;
		float newMaxZ = -INFINITY;

		for (int i = 0; i < points.size(); i++) {
			glm::vec4 p = t*points[i];

			if (p.x > newMaxX) newMaxX = p.x;
			if (p.y > newMaxY) newMaxY = p.y;
			if (p.z > newMaxZ) newMaxZ = p.z;
		}

		return glm::vec3(newMaxX, newMaxY, newMaxZ);
	}
	glm::vec3 Min(const glm::mat4 t) {

		float newMinX = INFINITY;
		float newMinY = INFINITY;
		float newMinZ = INFINITY;

		for (int i = 0; i < points.size(); i++) {
			glm::vec4 p = t*points[i];

			if (p.x < newMinX) newMinX = p.x;
			if (p.y < newMinY) newMinY = p.y;
			if (p.z < newMinZ) newMinZ = p.z;
		}

		return glm::vec3(newMinX, newMinY, newMinZ);
	}
};

struct Model{
  std::string name = "**UNNAMED Model**";
	int ID = -1;
	glm::mat4 transform;
	glm::mat4 modelOffset; //Just for placing geometry, not passed down the scene graph
	float* modelData = nullptr;
	int startVertex;
	int numVerts = 0;
	int numChildren = 0;
	int materialID = -1;
	Collider* collider = nullptr;
	glm::vec2 textureWrap = glm::vec2(1,1);
	glm::vec3 modelColor = glm::vec3(1,1,1); //TODO: Perhaps we can replace this simple approach with a more general material blending system?
	std::vector<Model*> childModel;

	Bounds* bounds = nullptr;
	int lodIndex = rand() % 4;
};

void resetModels();
void loadModel(string fileName);

void loadAllModelsTo1VBO(unsigned int vbo);
int addModel(string modelName);
void addChild(string childName, int curModelID);

//Global Model List
extern Model models[100000];
extern int numModels;

#endif //MODELS_H