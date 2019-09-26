#include "Shadows.h"
#include "RenderingCore.h"
#include "GPU-Includes.h"
#include "Shader.h" 

GLuint shadowVAO, shadowVBO, depthPosAttrib;
Shader depthShader;
unsigned int depthMapFBO, depthMapTex;
unsigned int shadowMapWidth = 1024, shadowMapHeight = 1024;

using std::vector;

int totalShadowTriangles = 0;

void initShadowMapping(){
	depthShader = Shader("shaders/depth-vert.glsl", "shaders/depth-frag.glsl");
	depthShader.init();
}

void initShadowBuffers(){
	//Build a Vertex Array Object for the full screen Quad. This stores the VBO to shader attribute mappings
	glGenVertexArrays(1, &shadowVAO); //Create a VAO
	glBindVertexArray(shadowVAO); //Bind the above created VAO to the current context

	GLuint shadowVBO; //We'll store all our models in one VBO //TODO: Compare to 1 VBO/Model
	glGenBuffers(1, &shadowVBO); 
	loadAllModelsTo1VBO(shadowVBO);

	depthPosAttrib = glGetAttribLocation(depthShader.ID, "position");
	glVertexAttribPointer(depthPosAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
	glEnableVertexAttribArray(depthPosAttrib);

  glBindVertexArray(0); //Unbind the VAO once we have set all the attributes	

	//Create a depthmap FBO
	glGenFramebuffers(1, &depthMapFBO);

	//Generate texture to store depth values
	glGenTextures(1, &depthMapTex);
	glBindTexture(GL_TEXTURE_2D, depthMapTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
							shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  //Assume anything outside of shadowmap is light (depth = max)

	//Bind the depth map texture to the FBO
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//A special draw call for shadow mapping because
//1. Not everything casts a shadow
//2. The shadow shader is much simpler (e.g, no color is needed)
void drawGeometryShadow(int shaderProgram, Model model, Material material, glm::mat4 transform){
	//printf("Model: %s, num Children %d\n",model.name.c_str(), model.numChildren);
	//printf("Material ID: %d\n", model.materialID);
	if (model.materialID >= 0) material = materials[model.materialID]; 

	transform *= model.transform;
	
	for (int i = 0; i < model.numChildren; i++){
		drawGeometryShadow(shaderProgram, *model.childModel[i], material, transform);
	}
	
	if (!model.modelData) return;

	transform *= model.modelOffset;
	GLint uniModelMatrixShadow = glGetUniformLocation(depthShader.ID, "model");
	glUniformMatrix4fv(uniModelMatrixShadow, 1, GL_FALSE, glm::value_ptr(transform));

	//printf("start/end %d %d\n",model.startVertex, model.numVerts);
	totalShadowTriangles += model.numVerts/3;
	glDrawArrays(GL_TRIANGLES, model.startVertex, model.numVerts); //(Primitive Type, Start Vertex, End Vertex) //Draw only 1st object
}

void computeShadowDepthMap(glm::mat4 lightView, glm::mat4 lightProjection, vector<Model*> toDrawShadows){
	glBindVertexArray(shadowVAO);
	depthShader.bind();
	glUniformMatrix4fv(glGetUniformLocation(depthShader.ID, "view"), 1, GL_FALSE, &lightView[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(depthShader.ID, "proj"), 1, GL_FALSE, &lightProjection[0][0]);

	glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	totalShadowTriangles = 0;
	
	//TODO: Let the user enable front-face culling for shadows if all models are closed
	//glEnable(GL_CULL_FACE); glCullFace(GL_FRONT);
	glm::mat4 I;
	for (size_t i = 0; i < toDrawShadows.size(); i++){
		//TODO: Allow some objects to not cast shadows @HW
		drawGeometryShadow(depthShader.ID, *toDrawShadows[i], materials[0], I);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
}