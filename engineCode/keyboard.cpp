#include "keyboard.h"
#include "luaSupport.h"
#include <external/loguru.hpp>

struct KeyState{
	bool up, down, right, left;
	bool shift, tab, space; 
  bool q,w,e,a,s,d,z,x,c;
};

struct KeyState keys;

void updateKeyboardState(){
  //List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
	//Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)

  const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
  keys.tab = keyboardState[SDL_SCANCODE_TAB];
  keys.up = keyboardState[SDL_SCANCODE_UP];
  keys.down = keyboardState[SDL_SCANCODE_DOWN];
  keys.left = keyboardState[SDL_SCANCODE_LEFT];
  keys.right = keyboardState[SDL_SCANCODE_RIGHT];
  keys.space = keyboardState[SDL_SCANCODE_SPACE];
  keys.shift = keyboardState[SDL_SCANCODE_LSHIFT] || keyboardState[SDL_SCANCODE_RSHIFT];
  keys.q = keyboardState[SDL_SCANCODE_Q];
  keys.w = keyboardState[SDL_SCANCODE_W];
  keys.e = keyboardState[SDL_SCANCODE_E];
  keys.a = keyboardState[SDL_SCANCODE_A];
  keys.s = keyboardState[SDL_SCANCODE_S];
  keys.d = keyboardState[SDL_SCANCODE_D];
  keys.z = keyboardState[SDL_SCANCODE_Z];
  keys.x = keyboardState[SDL_SCANCODE_X];
  keys.c = keyboardState[SDL_SCANCODE_C];
}

void keyboardUpdateLau(lua_State* L){
  //Call special keyHandler function in lua
		//---------------
		lua_getglobal(L, "keyHandler");
		lua_newtable(L);
		lua_pushboolean(L, keys.up); lua_setfield(L, -2, "up");
		lua_pushboolean(L, keys.down); lua_setfield(L, -2, "down");
		lua_pushboolean(L, keys.right); lua_setfield(L, -2, "right");
		lua_pushboolean(L, keys.left); lua_setfield(L, -2, "left");
		lua_pushboolean(L, keys.shift); lua_setfield(L, -2, "shift");
		lua_pushboolean(L, keys.tab); lua_setfield(L, -2, "tab");
    lua_pushboolean(L, keys.space); lua_setfield(L, -2, "space");
    lua_pushboolean(L, keys.q); lua_setfield(L, -2, "q");
    lua_pushboolean(L, keys.w); lua_setfield(L, -2, "w");
    lua_pushboolean(L, keys.e); lua_setfield(L, -2, "e");
    lua_pushboolean(L, keys.a); lua_setfield(L, -2, "a");
    lua_pushboolean(L, keys.s); lua_setfield(L, -2, "s");
    lua_pushboolean(L, keys.d); lua_setfield(L, -2, "d");
		lua_pushboolean(L, keys.z); lua_setfield(L, -2, "z");
    lua_pushboolean(L, keys.x); lua_setfield(L, -2, "x");
    lua_pushboolean(L, keys.c); lua_setfield(L, -2, "c");

		int luaErr = lua_pcall(L, 1, 0, 0);
		CHECK_F(luaErr==0, "Error after call to lua function 'keyHandler': %s \n", lua_tostring(L, -1));
}