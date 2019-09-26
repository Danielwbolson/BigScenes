#include <external/loguru.hpp>

#include "GPU-Includes.h"

//imgui includes
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

SDL_Window* sdlWindow;
SDL_Renderer* sdlRenderer;

int screenWidth, screenHeight;
int fullscreenMode = SDL_WINDOW_FULLSCREEN;

SDL_GLContext gl_context;

void setWindowSize(int w, int h, int fullscreen){
	LOG_SCOPE_FUNCTION(INFO);
	LOG_F(INFO,"Attempting to create a %s display of size %dx%d",fullscreen ? "Fullscreen": "Windowed",w,h);
	SDL_SetWindowFullscreen(sdlWindow, fullscreen ? fullscreenMode: 0); //Toggle fullscreen 
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  //This makes the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(sdlRenderer, w, h);
	SDL_GetWindowSize(sdlWindow, &screenWidth, &screenHeight);
	LOG_F(INFO,"Created a window of size %dx%d",screenWidth, screenHeight);
	LOG_IF_F(WARNING, screenWidth != w || screenHeight != h, "Actual screen size does not match requested screen size");
	//TODO: If the screen resolution changes we should recreate the textures sizes for the final composite
}

void createOpenGLWindow(int w, int h, int fullscreen){
  //Ask SDL to get a recent version of OpenGL (3.3 or greater)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);//
	
	//Create a window (offset_x, offset_y, width, height, flags)
	sdlWindow = SDL_CreateWindow("GEFS - Game Engine", 100, 100, w, h, SDL_WINDOW_OPENGL);
	
	//Create a context to draw in
	gl_context = SDL_GL_CreateContext(sdlWindow);
	
	//Lo}ad OpenGL extentions with GLAD
	bool openGLLoaded = gladLoadGLLoader(SDL_GL_GetProcAddress);
	CHECK_F(openGLLoaded, "ERROR: Failed to initialize OpenGL context.\n");
	
	//Log OpenGL Version (this should be at least 3.3)
	LOG_F(INFO,"\nOpenGL loaded\n");
	LOG_F(INFO,"Vendor:   %s\n", glGetString(GL_VENDOR));
	LOG_F(INFO,"Renderer: %s\n", glGetString(GL_RENDERER));
	LOG_F(INFO,"Version:  %s\n\n", glGetString(GL_VERSION));

	//Get SDL Renderer (will be OpenGL)  
	sdlRenderer = SDL_GetRenderer(sdlWindow);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.

  setWindowSize(w, h, fullscreen); 
}

void windowCleanup(){
  SDL_GL_DeleteContext(gl_context);
	SDL_Quit();
}

void initIMGui(){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	ImGui_ImplSDL2_InitForOpenGL(sdlWindow, gl_context);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImGui::StyleColorsDark();  //Try instead: ImGui::StyleColorsClassic();
}


// Start the ImGui frame
void IMGuiNewFrame(){
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(sdlWindow);
  ImGui::NewFrame();
}

//Double buffering
void swapDisplayBuffers(){
	SDL_GL_SwapWindow(sdlWindow);
}