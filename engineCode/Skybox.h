#ifndef SKYBOX
#define SKYBOX

extern unsigned int cubemapTexture;

//Skybox drawing functions
void initSkyboxShader();
void initSkyboxBuffers();
void updatePRBShaderSkybox();
void drawSkybox(glm::mat4 view, glm::mat4 proj);
void loadSkyboxToGPU();

#endif //SKYBOX