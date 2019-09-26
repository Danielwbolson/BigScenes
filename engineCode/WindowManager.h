#ifndef WINDOW_MANAGER
#define WINDOW_MANAGER

//SDL Window Management
void createOpenGLWindow(int w, int h, int fullscreen);
void setWindowSize(int w, int h, int fullscreen);
void swapDisplayBuffers();
void windowCleanup();

//IMGui Management
void initIMGui();
void IMGuiNewFrame();

extern int screenWidth, screenHeight;
extern int fullscreenMode; 

#endif //WINDOW_MANAGER