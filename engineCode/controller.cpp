
#include "controller.h"
#include "luaSupport.h"
#include <external/loguru.hpp>

SDL_GameController *sdlControllers[MAX_CONTROLLERS];
SDL_Haptic* sdlHaptics[MAX_CONTROLLERS] = {};
int numControllers = 0;

GamepadState gamepad; //TODO: We probably should have an array of MAX_CONTROLLERS gamepads

void initControllers(){
  int MaxJoysticks = SDL_NumJoysticks();
	bool error = false;
  numControllers = 0;
	//error =  SDL_GameControllerAddMappingsFromFile("./resourceFiles/gamecontrollerdb.txt"); //TODO: Use this list of common controller configurations
	//error  = SDL_GameControllerAddMappingsFromFile("./resourceFiles/usergamecontrollerdb.txt"); //TODO: allow user to configure game controller
	for(int JoystickIndex=0; JoystickIndex < MaxJoysticks; ++JoystickIndex){
		if (!SDL_IsGameController(JoystickIndex)) continue; //Skip joysticks
		if (numControllers >= MAX_CONTROLLERS) break; //Support only the first 4 controllers
    numControllers++;
		int controllerIndex = numControllers - 1;
    sdlControllers[controllerIndex] = SDL_GameControllerOpen(JoystickIndex);
		const char* controllerName = SDL_GameControllerName(sdlControllers[JoystickIndex]);
		LOG_F(INFO,"Controller Found: %s (Controller #%d)",controllerName,controllerIndex);

		//It seems controlpads don't have haptics in SDL only joysticks...
		SDL_Joystick* joystick = SDL_GameControllerGetJoystick(sdlControllers[controllerIndex]);
		if (SDL_JoystickIsHaptic(joystick)){
			printf("Controller Haptics Supported\n");
			sdlHaptics[controllerIndex] = SDL_HapticOpenFromJoystick(joystick);
			int err = SDL_HapticRumbleInit(sdlHaptics[controllerIndex]);
			if (err){ // haptic failed
				LOG_F(WARNING,"ERROR: Failed to initalized haptic -- %s\n",SDL_GetError());
				SDL_HapticClose(sdlHaptics[controllerIndex]);
				sdlHaptics[controllerIndex] = nullptr;
			}
		}
	}
	if (error){
		printf("ERROR: Couldn't load controller file\n");
		exit(1);
	}
	SDL_Joystick* joystick = SDL_GameControllerGetJoystick(sdlControllers[0]);
	if (SDL_IsGameController(0) && SDL_JoystickIsHaptic(joystick)){
		int a = SDL_HapticRumblePlay( sdlHaptics[0], 0.75, 500 );
		if (a) printf( "Warning: Unable to play rumble! %s\n", SDL_GetError() );
	}
}

void updateControllerState(){
  for (int c = 0; c < fmin(MAX_CONTROLLERS,numControllers); c++){ //TODO: Suppoort MAX_CONTROLLERS other than 1
    gamepad.attached = false;
    //printf("Reading Controller: %s\n",SDL_GameControllerName(sdlControllers[0]));
    if(sdlControllers[c] != 0 && SDL_GameControllerGetAttached(sdlControllers[c])){
      gamepad.attached = true;
      gamepad.up = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_DPAD_UP);
      gamepad.down = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_DPAD_DOWN);
      gamepad.left = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
      gamepad.right = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
      gamepad.start = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_START);
      gamepad.select = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_BACK);
      gamepad.lShoulder = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
      gamepad.rShoulder = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
      gamepad.AButton = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_A);
      gamepad.BButton = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_B);
      gamepad.XButton = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_X);
      gamepad.YButton = SDL_GameControllerGetButton(sdlControllers[c], SDL_CONTROLLER_BUTTON_Y);
      //printf("A: %d B: %d\n",AButton,BButton);

      gamepad.lstickX = float(SDL_GameControllerGetAxis(sdlControllers[c], SDL_CONTROLLER_AXIS_LEFTX))/ 32767.0f;
      gamepad.lstickY = float(SDL_GameControllerGetAxis(sdlControllers[c], SDL_CONTROLLER_AXIS_LEFTY))/ 32767.0f;
      gamepad.rstickX = float(SDL_GameControllerGetAxis(sdlControllers[c], SDL_CONTROLLER_AXIS_RIGHTX))/ 32767.0f;
      gamepad.rstickY = float(SDL_GameControllerGetAxis(sdlControllers[c], SDL_CONTROLLER_AXIS_RIGHTY))/ 32767.0f;
      gamepad.lTrigger = float(SDL_GameControllerGetAxis(sdlControllers[c], SDL_CONTROLLER_AXIS_TRIGGERLEFT))/ 32767.0f;
      gamepad.rTrigger = float(SDL_GameControllerGetAxis(sdlControllers[c], SDL_CONTROLLER_AXIS_TRIGGERRIGHT))/ 32767.0f;
      //printf("TT: %d %f\n",SDL_GameControllerGetAxis(sdlControllers[c], SDL_CONTROLLER_AXIS_TRIGGERLEFT),gamepad.lTrigger);
    } //else the controller is not plugged in
  }
}

void gamepadUpdateLua(lua_State* L){
  //TODO: Loop over all numControllers controllers
  if (SDL_GameControllerGetAttached(sdlControllers[0])){
    lua_getglobal(L, "gamepadUpdate");
    lua_newtable(L);
    lua_pushboolean(L, gamepad.up); lua_setfield(L, -2, "up");
    lua_pushboolean(L, gamepad.down); lua_setfield(L, -2, "down");
    lua_pushboolean(L, gamepad.right); lua_setfield(L, -2, "right");
    lua_pushboolean(L, gamepad.left); lua_setfield(L, -2, "left");
    lua_pushboolean(L, gamepad.lShoulder); lua_setfield(L, -2, "lShoulder");
    lua_pushboolean(L, gamepad.rShoulder); lua_setfield(L, -2, "rShoulder");
    lua_pushboolean(L, gamepad.start); lua_setfield(L, -2, "start");
    lua_pushboolean(L, gamepad.select); lua_setfield(L, -2, "select");
    lua_pushnumber(L, gamepad.lTrigger); lua_setfield(L, -2, "lTrigger");
    lua_pushnumber(L, gamepad.rTrigger); lua_setfield(L, -2, "rTrigger");
    lua_pushboolean(L, gamepad.AButton); lua_setfield(L, -2, "AButton");
    lua_pushboolean(L, gamepad.BButton); lua_setfield(L, -2, "BButton");
    lua_pushboolean(L, gamepad.XButton); lua_setfield(L, -2, "XButton");
    lua_pushboolean(L, gamepad.YButton); lua_setfield(L, -2, "YButton");
    lua_pushnumber(L, gamepad.lstickX); lua_setfield(L, -2, "lstickX");
    lua_pushnumber(L, gamepad.lstickY); lua_setfield(L, -2, "lstickY");
    lua_pushnumber(L, gamepad.rstickX); lua_setfield(L, -2, "rstickX");
    lua_pushnumber(L, gamepad.rstickY); lua_setfield(L, -2, "rstickY");

    int luaErr = lua_pcall(L, 1, 0, 0);
    CHECK_F(luaErr==0,"Error after 'gamepadUpdate': %s \n", lua_tostring(L, -1));
  }
}

void gamepadCleanup(){
  for(int ControllerIndex = 0; ControllerIndex < MAX_CONTROLLERS; ++ControllerIndex){
   if (sdlControllers[ControllerIndex]){
       SDL_GameControllerClose(sdlControllers[ControllerIndex]);
		}
	}
}