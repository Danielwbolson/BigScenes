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
	
		points.push_back(glm::vec4(maxX, maxY, maxZ, 1));
		points.push_back(glm::vec4(maxX, minY, maxZ, 1));
		points.push_back(glm::vec4(maxX, maxY, minZ, 1));
		points.push_back(glm::vec4(maxX, minY, minZ, 1));
		points.push_back(glm::vec4(minX, maxY, maxZ, 1));
		points.push_back(glm::vec4(minX, minY, maxZ, 1));
		points.push_back(glm::vec4(minX, maxY, minZ, 1));
		points.push_back(glm::vec4(minX, minY, minZ, 1));

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

		points.clear();

		for (int i = 0; i < b.points.size(); i++) {
			points.push_back(b.points[i]);
		}
		//points = b.points;

		return *this;
	}
	glm::vec3 Max(const glm::mat4 t) {

		float newMaxX = -INFINITY;
		float newMaxY = -INFINITY;
		float newMaxZ = -INFINITY;
		std::vector<glm::vec4> newPoints;

		for (int i = 0; i < points.size(); i++) {
			newPoints.push_back(t*points[i]);

			if (newPoints[i].x > newMaxX) newMaxX = newPoints[i].x;
			if (newPoints[i].y > newMaxY) newMaxY = newPoints[i].y;
			if (newPoints[i].z > newMaxZ) newMaxZ = newPoints[i].z;
		}

		return glm::vec3(newMaxX, newMaxY, newMaxZ);
	}
	glm::vec3 Min(const glm::mat4 t) {

		float newMinX = INFINITY;
		float newMinY = INFINITY;
		float newMinZ = INFINITY;
		std::vector<glm::vec4> newPoints;

		for (int i = 0; i < points.size(); i++) {
			newPoints.push_back(t*points[i]);

			if (newPoints[i].x < newMinX) newMinX = newPoints[i].x;
			if (newPoints[i].y < newMinY) newMinY = newPoints[i].y;
			if (newPoints[i].z < newMinZ) newMinZ = newPoints[i].z;
		}

		return glm::vec3(newMinX, newMinY, newMinZ);
	}
};

struct Model{
  std::string name = "**UNNAMED Model**";
	int ID = -1;
	glm::mat4 transform;
	glm::mat4 modelOffset; //Just for placing geometry, not passed down the scene graph
	float* modelData = 0;
	int startVertex;
	int numVerts = 0;
	int numChildren = 0;
	int materialID = -1;
	Collider* collider = nullptr;
	glm::vec2 textureWrap = glm::vec2(1,1);
	glm::vec3 modelColor = glm::vec3(1,1,1); //TODO: Perhaps we can replace this simple approach with a more general material blending system?
	std::vector<Model*> childModel;

	Bounds* bounds = nullptr;
};

void resetModels();
void loadModel(string fileName);

void loadAllModelsTo1VBO(unsigned int vbo);
int addModel(string modelName);
void addChild(string childName, int curModelID);

//Global Model List
extern Model models[4000];
extern int numModels;

#endif //MODELS_H