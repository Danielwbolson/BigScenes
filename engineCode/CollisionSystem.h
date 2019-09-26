#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "glm/glm.hpp"

#define MAX_LAYERS 10

#include <vector>
#include <string>

struct Collider{
  int ID;
	glm::vec3 offset;
  glm::vec3 globalPos;
  float r;
  int modelID;
  int layer;
};

int addCollider(int modelID, int layer, float r, glm::vec3 offset);
int getCollision(Collider* collider, int layer);
int getCollision(float x, float y, float z, float r, int layer);
void updateColliderPositions();

extern std::vector<int> collisionModels;
extern int numColliders[];

#endif //COLLISION_SYSTEM_H