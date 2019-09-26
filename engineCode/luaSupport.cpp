#include "luaSupport.h"
#include <algorithm>

#include <external/loguru.hpp>

#pragma GCC diagnostic ignored "-Wunused-variable" //TODO: Delete this line, this is hacky =/

void luaSetup(lua_State * L){
	//open all libraries
	luaL_openlibs(L);

	//register new custom method
	lua_register(L, "placeModel", placeModel);
	lua_register(L, "addModel", addModel);
	lua_register(L, "findModel", findModel);
	lua_register(L, "hideModel", hideModel);
	lua_register(L, "unhideModel", unhideModel);
	lua_register(L, "deleteModel", deleteModel);
	lua_register(L, "addCollider", addCollider);
	lua_register(L, "getCollisionsWithLayer", getCollisionsWithLayer);
	lua_register(L, "setModelColor", setModelColor);
	lua_register(L, "rotateModel", rotateModel);
	lua_register(L, "scaleModel", scaleModel);
	lua_register(L, "translateModel", translateModel);
	lua_register(L, "resetTransformation", resetTransformation);
	lua_register(L, "setModelMaterial", setModelMaterial);
	lua_register(L, "playSong", playSong);
	lua_register(L, "pauseSound", pauseSound);
	lua_register(L, "unpauseSound", unpauseSound);
	lua_register(L, "playSoundEffect", playSoundEffect);
	lua_register(L, "loadAudio", loadAudio);
}

//----------- Camera ----------

glm::vec3 getCameraPosFromLau(lua_State * L){
	glm::vec3 cameraPos;
	int argc = lua_gettop(L);
	lua_getglobal(L, "CameraPosX");
	cameraPos.x = (float)lua_tonumber(L, 1);
	lua_getglobal(L, "CameraPosY");
	cameraPos.y = (float)lua_tonumber(L, 2);
	lua_getglobal(L, "CameraPosZ");
	cameraPos.z = (float)lua_tonumber(L, 3);
	lua_pop(L, 3);
	return cameraPos;
}

glm::vec3 getCameraDirFromLau(lua_State * L){
	glm::vec3 cameraDir;
	int argc = lua_gettop(L);
	lua_getglobal(L, "CameraDirX");
	cameraDir.x = (float)lua_tonumber(L, 1);
	lua_getglobal(L, "CameraDirY");
	cameraDir.y = (float)lua_tonumber(L, 2);
	lua_getglobal(L, "CameraDirZ");
	cameraDir.z = (float)lua_tonumber(L, 3);
	lua_pop(L, 3);
	return cameraDir;
}

glm::vec3 getCameraUpFromLau(lua_State * L){
	glm::vec3 cameraUp;
	int argc = lua_gettop(L);
	lua_getglobal(L, "CameraUpX");
	cameraUp.x = (float)lua_tonumber(L, 1);
	lua_getglobal(L, "CameraUpY");
	cameraUp.y = (float)lua_tonumber(L, 2);
	lua_getglobal(L, "CameraUpZ");
	cameraUp.z = (float)lua_tonumber(L, 3);
	lua_pop(L, 3);
	return cameraUp;
}


//------------------- Audio ------------------------

//TODO: Maybe take in audio volume?
//TODO: Maybe add set (global) volume
//TODO: Add song path to setting.txt file
int loadAudio(lua_State * L){ //plays background music
	const char* rawSoundFileName;
	int argc = lua_gettop(L);
	rawSoundFileName = lua_tostring(L, 1);
	LOG_F(INFO,"Adding Audio Asset '%s'",rawSoundFileName);
	int audioID = audioManager.loadAudio(rawSoundFileName);

	lua_pushnumber(L, audioID);
	return 1;
}

int playSong(lua_State * L){ //plays background music
	int audioID = -1;
	int argc = lua_gettop(L);
	audioID = (int)lua_tonumber(L, 1);
	int volume = 120;
	if (argc > 1) volume = lua_tonumber(L, 2);
	LOG_F(INFO,"Playing Song ID '%d with volume %d'",audioID,volume); //TODO: Store the name of the song somewhere
	audioManager.playSong(audioID,volume);
	return 1;
}

int playSoundEffect(lua_State * L){ //plays background music
	int audioID = -1;
	int argc = lua_gettop(L);
	audioID = (int)lua_tonumber(L, 1);
	int volume = 120;
	if (argc > 1) volume = lua_tonumber(L, 2);
	float delay = 0;
	if (argc > 2) delay = lua_tonumber(L, 3);
	LOG_F(INFO,"Playing Sound Effect ID '%d'",audioID); //TODO: Store the name of the sound effect somewhere
	audioManager.playSoundEffect(audioID,volume,delay);
	return 1;
}

int pauseSound(lua_State * L){ //plays background music
	int argc = lua_gettop(L);
	LOG_F(INFO,"Puasing all sound!");
	audioManager.pause();
	return 0;
}

int unpauseSound(lua_State * L){ //plays background music
	int argc = lua_gettop(L);
	LOG_F(INFO,"Unpuasing all sound!");
	audioManager.unpause();
	return 0;
}

// -----------Model Loading ------------
#include <string>
using std::string;
using std::vector;
using std::swap; //fast delete

#include "Scene.h"
#include "Models.h"

vector<Model*> modelPool;

int addModel(lua_State * L){
	static int luaModelCount = 0;

	const char* rawModelName;
	float tx, ty, tz;
	int argc = lua_gettop(L);
	rawModelName = lua_tostring(L, 1);
	tx = lua_tonumber(L, 2);
	ty = lua_tonumber(L, 3);
	tz = lua_tonumber(L, 4);
	LOG_F(1,"Adding model %s at (%f, %f, %f)",rawModelName,tx,ty,tz);

	string childModelName(rawModelName);

	int myModelID;
	Model* pooledModel = 0;
	for (auto it = modelPool.begin(); it < modelPool.end(); it++){ //TODO: This is slow, maybe we should hash the row model name to an address somehow?
		if ((*it)->numChildren == 1 && (*it)->childModel[0]->name == childModelName){
			pooledModel = *it;
			myModelID = (*it)->ID;
			swap(*it, modelPool.back());
			modelPool.pop_back();
			//printf("Found model id %d (%d available) with only child named: %s",myModelID,modelPool.size(),childModelName.c_str());
			//TODO: We should ideally reset his material, etc... (not just transformation)
			models[myModelID].transform = glm::translate(glm::mat4(), glm::vec3(tx,ty,tz));
			break;
		}
	}

	if (!pooledModel){
		//printf("No pooled %s resource found (%d available), adding model",childModelName.c_str(),modelPool.size());
		string modelName = childModelName + std::to_string(luaModelCount);
		luaModelCount++;
		myModelID = addModel(modelName);
		addChild(childModelName,myModelID);
		models[myModelID].transform = glm::translate(models[myModelID].transform, glm::vec3(tx,ty,tz));
	}

	curScene.toDraw.push_back(&models[myModelID]);

	lua_pushnumber(L, myModelID);

	return 1;
}

#include "CollisionSystem.h"

extern vector<int> collisionModels;
extern int numColliders[MAX_LAYERS];
struct Collider;
extern Collider colliders[MAX_LAYERS][1000];

//TODO: Make offset optional 
int addCollider(lua_State * L){
	static int luaModelCount = 0;

	int modelID, layer;
	float tx, ty, tz, r;
	int argc = lua_gettop(L);
	modelID = (int)lua_tonumber(L, 1);
	layer = (int)lua_tonumber(L, 2);
	r = lua_tonumber(L, 3);
	tx = lua_tonumber(L, 4);
	ty = lua_tonumber(L, 5);
	tz = lua_tonumber(L, 6);
	LOG_F(INFO,"Adding collider to model %d in layer %d  (offest %f, %f, %f) with radius %f",modelID,layer,tx,ty,tz,r);

	int colliderID = addCollider(modelID,layer,r,glm::vec3(tx,ty,tz));

	lua_pushnumber(L, colliderID);

	return 1;
}

int getCollisionsWithLayer(lua_State * L){
	int layer;
	int argc = lua_gettop(L);

	int colliderModelID;
	if (argc == 2){
		int modelID;
		modelID = (int)lua_tonumber(L, 1);
		layer = (int)lua_tonumber(L, 2);
		colliderModelID = getCollision(models[modelID].collider,layer);
	}
	else{
		float x,y,z,r;
		x = lua_tonumber(L, 1);
		y = lua_tonumber(L, 2);
		z = lua_tonumber(L, 3);
		r = lua_tonumber(L, 4);
		layer = (int)lua_tonumber(L, 5);
		colliderModelID = getCollision(x,y,z,r,layer);
	}
	if (colliderModelID < 0) return 0; //No return values
	
	lua_pushinteger(L, colliderModelID);
	
	return 1;
}

int findModel(lua_State * L){
	int modelID = -1;
	string modelName;
	int argc = lua_gettop(L);
	modelName = lua_tostring(L, 1);
	for (int i = 0; i < numModels; i++){
		if (models[i].name == modelName){
			modelID = i;
			continue;
		} 
	}
	if (modelID == -1){
		LOG_F(WARNING,"ERROR: Model '%s' not found!",modelName.c_str());
		return 0; //Returns 'nil' to script
	}
	
	LOG_F(INFO,"Found Model '%s' at Node: %d",models[modelID].name.c_str(), modelID);

	lua_pushinteger(L, modelID);

	return 1;
}

int getChild(lua_State * L){
	int parentModelID, childNumber, childModelID;
	string childName;
	int argc = lua_gettop(L);
	parentModelID = (int)lua_tonumber(L, 1);
	childNumber = (int)lua_tonumber(L, 2);
	LOG_F(INFO,"Finding Child: %d of %d children",childNumber,(int)models[parentModelID].childModel[0]->childModel.size());
	childName = models[parentModelID].childModel[0]->childModel[childNumber]->name;
	for (int i = 0; i < numModels; i++){
		if (models[i].name == childName){
			childModelID = i;
			continue;
		} 
	}
	LOG_F(INFO,"Node %s child #%d is %s",models[parentModelID].name.c_str(), childNumber, models[childModelID].name.c_str());

	return 1;
}

int deleteModel(lua_State * L){
	int modelID;
	int argc = lua_gettop(L);
	modelID = (int)lua_tonumber(L, 1);
	LOG_F(INFO,"Deleting model %s with ID: %d",models[modelID].name.c_str(), modelID);

	auto it = std::find(curScene.toDraw.begin(),curScene.toDraw.end(),&models[modelID]); //TODO: This is slow, maybe we should track toDraw index for fast delete @performance
	if (it != curScene.toDraw.end()) { //TODO: No need to do this check if we only call delete on existing models
		swap(*it, curScene.toDraw.back());
		modelPool.push_back(curScene.toDraw.back());
		//printf("Delete found model with child named: %s",(curScene.toDraw.back())->childModel[0]->name.c_str());
		curScene.toDraw.pop_back();
	}
	else{
		LOG_F(WARNING,"ERROR: Model is already Deleted.");
	}

	//TODO: Allow a model to have multiple colliders
	//TODO: Allow a child node have a collider (e.g., find global position of child, probably still point collision to parent though)
	auto cModel = std::find(collisionModels.begin(),collisionModels.end(),modelID); //TODO: This is slow, maybe I should track collisionModel index for fast delete
	if (cModel != collisionModels.end()) { //TODO: No need to do this check if we only call delete on existing models
		swap(*cModel, collisionModels.back());
		collisionModels.pop_back();

		int layer = models[modelID].collider->layer;
		numColliders[layer]--; //Reduce collider count
		int collidersInLayer = numColliders[layer];
		for (int i = 0; i < collidersInLayer; i++){
			if (colliders[layer][i].modelID == modelID){
				colliders[layer][i] = colliders[layer][collidersInLayer];
				models[colliders[layer][i].modelID].collider = &colliders[layer][i];
				break;
			}
		}
	}
	else{
		LOG_F(WARNING,"ERROR: Model is already Deleted.");
	}

	return 1;
}

int hideModel(lua_State * L){
	int modelID;
	int argc = lua_gettop(L);
	modelID = (int)lua_tonumber(L, 1);
	LOG_F(1,"No longer drawing model '%s' with ID: %d",models[modelID].name.c_str(), modelID);

	auto it = std::find(curScene.toDraw.begin(),curScene.toDraw.end(),&models[modelID]); //TODO: This is slow, maybe I should track curScene.toDraw index for fast delete
	if (it != curScene.toDraw.end()) { //TODO: No need to do this check if we are safe
		swap(*it, curScene.toDraw.back());
		curScene.toDraw.pop_back();
	}
	else{
		LOG_F(WARNING,"ERROR: Model is already hidden.");
	}

	return 1;
}

int unhideModel(lua_State * L){
	int modelID;
	int argc = lua_gettop(L);
	modelID = (int)lua_tonumber(L, 1);
	LOG_F(1,"No longer hiding model %s with ID: %d",models[modelID].name.c_str(), modelID);
	auto it = std::find(curScene.toDraw.begin(),curScene.toDraw.end(),&models[modelID]); //TODO: This is slow, maybe we should track toDraw index for fast delete
	if (it == curScene.toDraw.end()) { //TODO: No need to do this check if we are safe
		curScene.toDraw.push_back(&models[modelID]);
	}

	return 1;
}

int scaleModel(lua_State * L){
	int modelID = -1;
	float sx, sy, sz;
	int argc = lua_gettop(L);
	modelID = lua_tonumber(L, 1);
	sx = lua_tonumber(L, 2);
	sy = lua_tonumber(L, 3);
	sz = lua_tonumber(L, 4);
	LOG_F(1,"Scaling model %s at (%f, %f, %f)",models[modelID].name.c_str(),sx,sy,sz);

	models[modelID].transform = glm::scale(models[modelID].transform, glm::vec3(sx,sy,sz));

	return 0;
}

int setModelMaterial(lua_State * L){
	int modelID = -1;
	int argc = lua_gettop(L);
	modelID = lua_tonumber(L, 1);
	string materialName = string(lua_tostring(L, 2));
	int newMaterial = findMaterial(materialName);
	LOG_F(1,"Setting model %s material to '%s'",models[modelID].name.c_str(),materials[newMaterial].name.c_str());

	models[modelID].materialID = newMaterial;

	return 0;
}

int setModelColor(lua_State * L){
	int modelID = -1;
	float r, g, b;
	int argc = lua_gettop(L);
	modelID = lua_tonumber(L, 1);
	r = lua_tonumber(L, 2);
	g = lua_tonumber(L, 3);
	b = lua_tonumber(L, 4);
	LOG_F(1,"Setting model ID %s to color (%f, %f, %f)",models[modelID].name.c_str(),r,g,b);

	models[modelID].modelColor = glm::vec3(r,g,b);

	return 0;
}

//TODO: Placing a model undoes its rotation/scale, we should probably fix this
int placeModel(lua_State * L){
	int modelID = -1;
	float tx, ty, tz;
	int argc = lua_gettop(L);
	modelID = lua_tonumber(L, 1);
	tx = lua_tonumber(L, 2);
	ty = lua_tonumber(L, 3);
	tz = lua_tonumber(L, 4);
	LOG_F(1,"Placing model %s at (%f, %f, %f)",models[modelID].name.c_str(),tx,ty,tz);

	models[modelID].transform = glm::translate(glm::mat4(), glm::vec3(tx,ty,tz));

	return 0;
}

//Give a position and rotation
int placeModelAtAngle(lua_State * L){
	int modelID = -1;
	float tx, ty, tz;
	float ang, rx, ry, rz;
	int argc = lua_gettop(L);
	modelID = lua_tonumber(L, 1);
	tx = lua_tonumber(L, 2);
	ty = lua_tonumber(L, 3);
	tz = lua_tonumber(L, 4);
	ang = lua_tonumber(L, 5);
	rx = lua_tonumber(L, 6);
	ry = lua_tonumber(L, 7);
	rz = lua_tonumber(L, 8);
	LOG_F(1,"Placing model %s at (%f, %f, %f)",models[modelID].name.c_str(),tx,ty,tz);

	models[modelID].transform = glm::translate(glm::mat4(), glm::vec3(tx,ty,tz));
	models[modelID].transform = glm::rotate(models[modelID].transform, ang, glm::vec3(rx,ry,rz));

	return 0;
}

//TODO: This assumes you will translate after rotating, is that safe?
int translateModel(lua_State * L){
	int modelID = -1;
	float tx, ty, tz;
	int argc = lua_gettop(L);
	modelID = lua_tonumber(L, 1);
	tx = lua_tonumber(L, 2);
	ty = lua_tonumber(L, 3);
	tz = lua_tonumber(L, 4);
	LOG_F(1,"Translating model %s at (%f, %f, %f)",models[modelID].name.c_str(),tx,ty,tz);

	glm::mat4 tmat = glm::translate(glm::mat4(), glm::vec3(tx,ty,tz));
	models[modelID].transform = tmat*models[modelID].transform;


	return 0;
}

int rotateModel(lua_State * L){
	int modelID = -1;
	float ang, rx, ry, rz;
	int argc = lua_gettop(L);
	modelID = lua_tonumber(L, 1);
	ang = lua_tonumber(L, 2);
	rx = lua_tonumber(L, 3);
	ry = lua_tonumber(L, 4);
	rz = lua_tonumber(L, 5);
	LOG_F(1,"Rotating model %s at by angle %f along (%f, %f, %f)",models[modelID].name.c_str(), ang, rx,ry,rz);

	//printf("# Children: %d",models[modelID].numChildren;);

	models[modelID].transform = glm::rotate(models[modelID].transform, ang, glm::vec3(rx,ry,rz));

	return 0;
}

int resetTransformation(lua_State * L){
	int modelID = -1;
	float ang, rx, ry, rz;
	int argc = lua_gettop(L);
	modelID = lua_tonumber(L, 1);
	ang = lua_tonumber(L, 2);
	rx = lua_tonumber(L, 3);
	ry = lua_tonumber(L, 4);
	rz = lua_tonumber(L, 5);
	LOG_F(1,"Resetting model %s transform to Identity )",models[modelID].name.c_str());

	models[modelID].transform = glm::mat4();

	return 0;
}