#ifndef GPU_INCLUDES_H
#define GPU_INCLUDES_H

#include "glad/glad.h"  //Include order can matter here

#if defined(__linux__)
 #include <SDL2/SDL.h>
 #include <SDL2/SDL_opengl.h>
#else
 #include <SDL.h>
 #include <SDL_opengl.h>
#endif

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#endif //GPU_INCLUDES_H