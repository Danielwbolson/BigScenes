#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "GPU-Includes.h"

#define MAX_CONTROLLERS 1 //TODO: Support more than 1 controller

struct lua_State; //this will be defined elsewhere (in luaSupport.cpp)

void initControllers();
void updateControllerState();
void gamepadUpdateLua(lua_State* L);
void gamepadCleanup(); 

struct GamepadState{
	bool attached = false;
	bool up = false;
	bool down = false;
	bool right = false;
	bool left = false;
	float lTrigger = 0;
	float rTrigger = 0;
	bool lShoulder = false;
	bool rShoulder = false;
	bool select = false;
	bool start = false;
	bool AButton = false;
	bool BButton = false;
	bool XButton = false;
	bool YButton = false;
	float lstickX = 0; 
	float lstickY = 0;
	float rstickX = 0; 
	float rstickY = 0;  
};

#endif //CONTROLLER_H