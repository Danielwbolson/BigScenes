#include <vector>

//HDR Buffers and textures
extern unsigned int pingpongFBO[2];
extern unsigned int pingpongColorbuffers[2];
extern unsigned int baseTex, brightText; //Textures which are bound to the HDR FBO

//Full screen quad rendering
extern unsigned int quadVAO;
void createFullscreenQuad();
