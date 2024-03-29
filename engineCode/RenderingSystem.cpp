#include "GPU-Includes.h"
#include "RenderingSystem.h"
#include "RenderingCore.h"
#include "WindowManager.h"
#include "Bloom.h"
#include "Skybox.h"
#include "Shadows.h"
#include "CollisionSystem.h"
#include "Shader.h"
#include <external/loguru.hpp>
#include <external/stb_image.h>
#include <algorithm>


using std::vector;

GLuint tex[1000];

bool xxx; //Just an unspecified bool that gets passed to shader for debugging

int sphereVerts; //Number of verts in the colliders spheres

int totalTriangles = 0;

Bounds bounds;

GLint uniColorID, uniEmissiveID, uniUseTextureID, modelColorID;
GLint metallicID, roughnessID, iorID, reflectivenessID;
GLint uniModelMatrix, colorTextureID, texScaleID, biasID, pcfID;
GLint xxxID;

GLuint colliderVAO; //Build a Vertex Array Object for the collider

void drawGeometry(Model model, int matID, glm::mat4 transform = glm::mat4(), const glm::mat4& projViewMat = glm::mat4(),const int& lodIndex = 3,glm::vec2 textureWrap=glm::vec2(1,1), glm::vec3 modelColor=glm::vec3(1,1,1));
bool frustumCull(const Model& model, const glm::mat4& transform, const glm::mat4& projViewMat);
void assignLod(Model model, const glm::mat4& projMat, const glm::mat4& viewMat, int* lodIndex, glm::mat4 transform = glm::mat4());

void drawGeometry(Model model, int materialID, glm::mat4 transform, const glm::mat4& projViewMat, const int& lodIndex, glm::vec2 textureWrap, glm::vec3 modelColor){
	//printf("Model: %s, num Children %d\n",model.name.c_str(), model.numChildren);
	//printf("Material ID: %d (passed in id = %d)\n", model.materialID,materialID);
	//printf("xyx %f %f %f %f\n",model.transform[0][0],model.transform[0][1],model.transform[0][2],model.transform[0][3]);
	//if (model.materialID >= 0) material = materials[model.materialID];

	Material material;
	if (materialID < 0){
		materialID = model.materialID; 
	} 
	if (materialID >= 0){
		material = materials[materialID]; // Maybe should be a pointer
	}
	//printf("Using material %d - %s\n", model.materialID,material.name.c_str());

	transform *= model.transform;
	modelColor *= model.modelColor;
	//textureWrap *= model.textureWrap; //TODO: Where best to apply textureWrap transform?
	
	for (int i = 0; i < model.numChildren; i++){
		drawGeometry(*model.childModel[i], materialID, transform, projViewMat, lodIndex, textureWrap, modelColor);
	}
	if (!model.modelData) return;
	
	transform *= model.modelOffset;
	textureWrap *= model.textureWrap; //TODO: Should textureWrap stack like this?

	if (model.bounds == nullptr) return;
	if (model.name.find("lod" + std::to_string(lodIndex)) == std::string::npos) return;

	if (frustumCull(model, transform, projViewMat)) return;


	glUniformMatrix4fv(uniModelMatrix, 1, GL_FALSE, glm::value_ptr(transform));

	glUniform1i(uniUseTextureID, material.textureID >= 0); //textureID of -1 --> no texture

	if (material.textureID >= 0){
		glActiveTexture(GL_TEXTURE0);  //Set texture 0 as active texture
		glBindTexture(GL_TEXTURE_2D, tex[material.textureID]); //Load bound texture
		glUniform1i(colorTextureID, 0); //Use the texture we just loaded (texture 0) as material color
		glUniform2fv(texScaleID, 1, glm::value_ptr(textureWrap));
	}

	glUniform1i(xxxID, xxx);

	//printf("%f\n",model.modelColor[0]);
	glUniform3fv(modelColorID, 1, glm::value_ptr(modelColor*model.modelColor)); //multiply parent's color by your own

	glUniform1f(metallicID, material.metallic); 
	glUniform1f(roughnessID, material.roughness); 
	glUniform1f(iorID, material.ior);
	glUniform1f(reflectivenessID, material.reflectiveness);
	glUniform3fv(uniColorID, 1, glm::value_ptr(material.col));
	glUniform3fv(uniEmissiveID, 1, glm::value_ptr(material.emissive));

	//printf("start/end %d %d\n",model.startVertex, model.numVerts);
	totalTriangles += model.numVerts/3; //3 verts to a triangle
	glDrawArrays(GL_TRIANGLES, model.startVertex, model.numVerts); //(Primitive Type, Start Vertex, End Vertex) //Draw only 1st object
}

void drawColliderGeometry(){ //, Material material //TODO: Take in a material for the colliders
	//printf("Drawing %d Colliders\n",collisionModels.size());
	//printf("Material ID: %d\n", material);

	glBindVertexArray(colliderVAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::vec3 baseColor = glm::vec3(1,1,1);
	glm::vec3 modelColor = glm::vec3(1,0,0);

	glUniform1i(uniUseTextureID, -1); //textureID of -1 --> no texture

	glUniform1i(xxxID, xxx);

	glUniform3fv(modelColorID, 1, glm::value_ptr(baseColor)); //multiply parent's color by your own

	glUniform1f(metallicID, 0); 
	glUniform1f(roughnessID, 0); 
	glUniform1f(iorID, 1);
	glUniform3fv(uniColorID, 1, glm::value_ptr(modelColor));
	glUniform3fv(uniEmissiveID, 1, glm::value_ptr(modelColor));
	
	//TODO: Maybe loop through each layer and color by layer instead?
	for (size_t i = 0; i < collisionModels.size(); i++){
		Collider* c = models[collisionModels[i]].collider;
		if (c == nullptr) continue;
		//printf("Drawing at pos %f %f %f, radius = %f\n",c->globalPos[0],c->globalPos[1],c->globalPos[2],c->r);

		glm::mat4 colliderTans = glm::translate(glm::mat4(), c->globalPos);
		colliderTans = glm::scale(colliderTans, glm::vec3(c->r,c->r,c->r));
		
		glUniformMatrix4fv(uniModelMatrix, 1, GL_FALSE, glm::value_ptr(colliderTans));
		glDrawArrays(GL_TRIANGLES, 0, sphereVerts/3); //(Primitive Type, Start Vertex, End Vertex) //Draw only 1st object
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool frustumCull(const Model& model, const glm::mat4& transform, const glm::mat4& pv) {
	//TODO: DANIEL --> Determine which models are even worth drawing
/* http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf */
	glm::vec4 left = glm::vec4(
		pv[0][3] + pv[0][0],
		pv[1][3] + pv[1][0],
		pv[2][3] + pv[2][0],
		pv[3][3] + pv[3][0]);

	glm::vec4 right = glm::vec4(
		pv[0][3] - pv[0][0],
		pv[1][3] - pv[1][0],
		pv[2][3] - pv[2][0],
		pv[3][3] - pv[3][0]);

	glm::vec4 bottom = glm::vec4(
		pv[0][3] + pv[0][1],
		pv[1][3] + pv[1][1],
		pv[2][3] + pv[2][1],
		pv[3][3] + pv[3][1]);

	glm::vec4 top = glm::vec4(
		pv[0][3] - pv[0][1],
		pv[1][3] - pv[1][1],
		pv[2][3] - pv[2][1],
		pv[3][3] - pv[3][1]);

	glm::vec4 nearPlane = glm::vec4(
		pv[0][2],
		pv[1][2],
		pv[2][2],
		pv[3][2]);

	glm::vec4 farPlane = glm::vec4(
		pv[0][3] - pv[0][2],
		pv[1][3] - pv[1][2],
		pv[2][3] - pv[2][2],
		pv[3][3] - pv[3][2]);

	Bounds b = *(model.bounds);
	glm::vec4 tbMax = glm::vec4(b.Max(transform), 1);
	glm::vec4 tbMin = glm::vec4(b.Min(transform), 1);
	glm::vec3 max = glm::vec3(tbMax.x, tbMax.y, tbMax.z);
	glm::vec3 min = glm::vec3(tbMin.x, tbMin.y, tbMin.z);

	std::vector<glm::vec3> cam_to_b = std::vector<glm::vec3>(8);
	cam_to_b[0] = glm::vec3(max.x, max.y, max.z);
	cam_to_b[1] = glm::vec3(max.x, min.y, max.z);
	cam_to_b[2] = glm::vec3(max.x, max.y, min.z);
	cam_to_b[3] = glm::vec3(max.x, min.y, min.z);
	cam_to_b[4] = glm::vec3(min.x, max.y, max.z);
	cam_to_b[5] = glm::vec3(min.x, min.y, max.z);
	cam_to_b[6] = glm::vec3(min.x, max.y, min.z);
	cam_to_b[7] = glm::vec3(min.x, min.y, min.z);

	for (int j = 0; j < cam_to_b.size(); j++) {
		int success = 0;
		success += (left.x * cam_to_b[j].x + left.y * cam_to_b[j].y + left.z * cam_to_b[j].z + left.w > 0);
		success += (right.x * cam_to_b[j].x + right.y * cam_to_b[j].y + right.z * cam_to_b[j].z + right.w > 0);
		success += (top.x * cam_to_b[j].x + top.y * cam_to_b[j].y + top.z * cam_to_b[j].z + top.w > 0);
		success += (bottom.x * cam_to_b[j].x + bottom.y	* cam_to_b[j].y + bottom.z * cam_to_b[j].z + bottom.w > 0);
		success += (nearPlane.x * cam_to_b[j].x + nearPlane.y * cam_to_b[j].y + nearPlane.z * cam_to_b[j].z + nearPlane.w > 0);
		success += (farPlane.x * cam_to_b[j].x + farPlane.y * cam_to_b[j].y + farPlane.z * cam_to_b[j].z + farPlane.w > 0);

		if (success == 6) {
			return false;
		}
	}

	return true;

}

void assignLod(Model model, const glm::mat4& projMat, const glm::mat4& viewMat, int* lodIndex, glm::mat4 transform) {

	transform *= model.transform;

	for (int i = 0; i < model.numChildren; i++) {
		assignLod(*model.childModel[i], projMat, viewMat, lodIndex, transform);
	}

	// Get bounds from the top lod and ignore the weird fake children stephen creates
	if (model.name.find("lod0") != std::string::npos && model.name.find("Child") == std::string::npos) {
		bounds = *model.bounds;
	}
	// Ignore the parent model by looking for model with 4 children (level of details)
	if (model.numChildren == 4) return;
	// We only want to work with the original model, not levels of detail
	if (model.name.find("lod") != std::string::npos) return;

	glm::vec4 max = projMat * glm::vec4(bounds.Max(viewMat * transform), 1);
	glm::vec4 min = projMat * glm::vec4(bounds.Min(viewMat * transform), 1);

	max /= max.w;
	min /= min.w;

	max.y = (max.y + 1) / 2.0;
	min.y = (min.y + 1) / 2.0;

	float screenMaxY = screenHeight * max.y; // (max.y + 1) / 2.0;
	float screenMinY = screenHeight * min.y; // (min.y + 1) / 2.0;

	float screenRatio = (screenMaxY - screenMinY) / screenHeight;

	// If a model is behind us or extends out of the window, make it lod0
	if (screenRatio < 0 || transform[3].z >= -(viewMat[3].z + 0.001f)) { *lodIndex = 0; } 
	else if (screenRatio < 0.01) { *lodIndex = 3; }
	else if (screenRatio < 0.25) { *lodIndex = 2; }
	else if (screenRatio < 0.5) { *lodIndex = 1; }
	else { *lodIndex =  0; }
}

//TODO: When is the best time to call this? This loads all textures, should we call it per-texture instead?
void loadTexturesToGPU(){
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);
  for (int i = 0; i < numTextures; i++){
    LOG_F(1,"Loading Texture %s",textures[i].c_str());
    unsigned char *pixelData = stbi_load(textures[i].c_str(), &width, &height, &nrChannels, STBI_rgb);
		CHECK_NOTNULL_F(pixelData,"Fail to load model texture: %s",textures[i].c_str()); //TODO: Is there some way to get the error from STB image?
    
		//Load the texture into memory
    glGenTextures(1, &tex[i]);
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, width, height); //Mipmap levels
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
		glGenerateMipmap(GL_TEXTURE_2D);
    
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //TODO: Does this look better? I'm not sure
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(pixelData);
	}
}


//------------ HDR ------------------
GLuint fboHDR; //floating point FBO for HDR rendering (two render outputs, base color and bloom)
GLuint baseTex, brightText; //Textures which are bound to the bloom FBOs
unsigned int pingpongFBO[2];
unsigned int pingpongColorbuffers[2];

void initHDRBuffers(){
	glGenFramebuffers(1, &fboHDR);
	glBindFramebuffer(GL_FRAMEBUFFER, fboHDR);

	glGenTextures(1, &baseTex);
	glBindTexture(GL_TEXTURE_2D, baseTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, baseTex, 0);
	
	glGenTextures(1, &brightText);
	glBindTexture(GL_TEXTURE_2D, brightText);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, brightText, 0);

	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	//Specify which color attachments we'll use (of this framebuffer) for rendering (both regular and bright pixels) 
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// finally check if framebuffer is complete
	CHECK_F(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// A pair of ping-pong framebuffers for extended blurring
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++){
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
			glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //why clamp?
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER,0); //Return to normal rendering
}


//------------ PBR Shader ---------
Shader PBRShader;

GLint posAttrib, texAttrib, normAttrib;
GLint uniView,uniInvView, uniProj;
GLuint modelsVAO, modelsVBO;

void initPBRShading(){
	PBRShader = Shader("shaders/vertexTex.glsl", "shaders/fragmentTex.glsl");
	PBRShader.init();

	//Build a Vertex Array Object. This stores the VBO to shader attribute mappings
	glGenVertexArrays(1, &modelsVAO); //Create a VAO
	glBindVertexArray(modelsVAO); //Bind the above created VAO to the current context

	//We'll store all our models in one VBO //TODO: We should compare to 1 VBO/model?
	glGenBuffers(1, &modelsVBO); 
	loadAllModelsTo1VBO(modelsVBO);

	//Tell OpenGL how to set fragment shader input 
	posAttrib = glGetAttribLocation(PBRShader.ID, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
	  //Attribute, vals/attrib., type, normalized?, stride, offset
	  //Binds to VBO current GL_ARRAY_BUFFER 
	glEnableVertexAttribArray(posAttrib);
	
	//GLint colAttrib = glGetAttribLocation(phongShader, "inColor");
	//glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(colAttrib);
	
	texAttrib = glGetAttribLocation(PBRShader.ID, "inTexcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

	normAttrib = glGetAttribLocation(PBRShader.ID, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(normAttrib);
	

	uniView = glGetUniformLocation(PBRShader.ID, "view");
	uniInvView  = glGetUniformLocation(PBRShader.ID, "invView"); //inverse of view matrix
	uniProj = glGetUniformLocation(PBRShader.ID, "proj");


	uniColorID = glGetUniformLocation(PBRShader.ID, "materialColor");
  uniEmissiveID = glGetUniformLocation(PBRShader.ID, "emissive");
  uniUseTextureID = glGetUniformLocation(PBRShader.ID, "useTexture");
	modelColorID = glGetUniformLocation(PBRShader.ID, "modelColor");
	metallicID = glGetUniformLocation(PBRShader.ID, "metallic");
	roughnessID = glGetUniformLocation(PBRShader.ID, "roughness");
	biasID = glGetUniformLocation(PBRShader.ID, "shadowBias");
	pcfID = glGetUniformLocation(PBRShader.ID, "pcfSize");
	iorID = glGetUniformLocation(PBRShader.ID, "ior");
	reflectivenessID = glGetUniformLocation(PBRShader.ID, "reflectiveness");
	uniModelMatrix = glGetUniformLocation(PBRShader.ID, "model");
	colorTextureID = glGetUniformLocation(PBRShader.ID, "colorTexture");
	texScaleID = glGetUniformLocation(PBRShader.ID, "textureScaleing");
	xxxID = glGetUniformLocation(PBRShader.ID, "xxx");

	PBRShader.bind();
	glUniformMatrix4fv(glGetUniformLocation(PBRShader.ID, "rotSkybox"), 1, GL_FALSE, &curScene.rotSkybox[0][0]);


	glBindVertexArray(0); //Unbind the VAO in case we want to create a new one
}

void setPBRShaderUniforms(glm::mat4 view, glm::mat4 proj, glm::mat4 lightViewMatrix, glm::mat4 lightProjectionMatrix, bool useShadowMap){
	glBindFramebuffer(GL_FRAMEBUFFER, fboHDR);
	PBRShader.bind();
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
	glm::mat4 invView = glm::inverse(view);
	glUniformMatrix4fv(uniInvView, 1, GL_FALSE, glm::value_ptr(invView));
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	glUniform1i(glGetUniformLocation(PBRShader.ID, "numLights"), curScene.lights.size());

	glUniform3fv(glGetUniformLocation(PBRShader.ID, "inLightDir"), curScene.lights.size(), glm::value_ptr(lightDirections[0]));
	glUniform3fv(glGetUniformLocation(PBRShader.ID, "inLightCol"), curScene.lights.size(), glm::value_ptr(lightColors[0]));

	glUniform1f(biasID, curScene.shadowLight.shadowBias);
	glUniform1i(pcfID, curScene.shadowLight.pcfWidth);

	glUniform3fv(glGetUniformLocation(PBRShader.ID, "ambientLight"), 1, glm::value_ptr(curScene.ambientLight));
	
	glUniformMatrix4fv(glGetUniformLocation(PBRShader.ID, "shadowView"), 1, GL_FALSE, &lightViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(PBRShader.ID, "shadowProj"), 1, GL_FALSE, &lightProjectionMatrix[0][0]);

	glActiveTexture(GL_TEXTURE1); //Texture 1 in the PBR Shader is the shadow map
	glBindTexture(GL_TEXTURE_2D, depthMapTex); 
	glUniform1i(glGetUniformLocation(PBRShader.ID, "shadowMap"), 1);

	glUniform1i(glGetUniformLocation(PBRShader.ID, "useShadow"), useShadowMap && curScene.shadowLight.castShadow);
}

// ---------------- Collider Geometry -----------

int createColliderSphere(int sphereVbo) {
	int stacks = 4;
	int slices = 4;
	const float PI = 3.14f;

	std::vector<float> positions;
	std::vector<float> verts;

	for (int i = 0; i <= stacks; ++i){
			float V = (float)i / (float)stacks; 
			float phi = V * PI;

			for (int j = 0; j <= slices; ++j){
					float U = (float)j / (float)slices;
					float theta = U * (PI * 2);

					// use spherical coordinates to calculate the positions.
					float x = cos(theta) * sin(phi);
					float y = cos(phi);
					float z = sin(theta) * sin(phi);

					positions.push_back(x);
					positions.push_back(y);
					positions.push_back(z);
			}
	}

	// Calc The Index Positions
	for (int i = 0; i < slices * stacks + slices; ++i){
		int s = i;
		for (int j = 0; j < 3; j++) verts.push_back(positions[3*s+j]);
		s = i + slices + 1;
		for (int j = 0; j < 3; j++) verts.push_back(positions[3*s+j]);
		s = i + slices;
		for (int j = 0; j < 3; j++) verts.push_back(positions[3*s+j]);

		s = i + slices + 1;
		for (int j = 0; j < 3; j++) verts.push_back(positions[3*s+j]);
		s = i;
		for (int j = 0; j < 3; j++) verts.push_back(positions[3*s+j]);
		s = i + 1;
		for (int j = 0; j < 3; j++) verts.push_back(positions[3*s+j]);
	}

	// upload geometry to GPU.
	glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), verts.data(), GL_STATIC_DRAW);

	LOG_F(INFO,"Created Colider Circle with %d verts", (int)verts.size());
	return (int)verts.size();
}


void initColliderGeometry(){
	//PBRShader.bind();  //It's still bound from our call to initPBRShading()
	glGenVertexArrays(1, &colliderVAO); //Create a VAO
	glBindVertexArray(colliderVAO); //Bind the above created VAO to the current context

	GLuint colliderVBO;
  glGenBuffers(1, &colliderVBO);  //Create 1 buffer called vbo
  glBindBuffer(GL_ARRAY_BUFFER, colliderVBO); //(Only one buffer can be bound at a time) 

	//Tell OpenGL how to set fragment shader input 
	//posAttrib = glGetAttribLocation(PBRShader.ID, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

	sphereVerts = createColliderSphere(colliderVBO);

  glBindVertexArray(0); //Unbind the VAO once we have set all the attributes
}


// --------  Draw Scene Geometry ----
void updatePRBShaderSkybox(){
	glActiveTexture(GL_TEXTURE5); //TODO: List what the first 5 textures are used for
	glUniform1i(glGetUniformLocation(PBRShader.ID, "skybox"),5);
	
	glUniform1i(glGetUniformLocation(PBRShader.ID, "useSkyColor"), curScene.singleSkyColor);
	if (curScene.singleSkyColor){
		glUniform3fv(glGetUniformLocation(PBRShader.ID, "skyColor"), 1, glm::value_ptr(curScene.skyColor));
	} else{
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	}
}

struct CompareDist {
	CompareDist(const glm::mat4 mat) : m(mat) {
		camPos = glm::vec3(-m[3].x, -m[3].y, -m[3].z);
	}

	bool operator ()(const Model* a, const Model* b) {
		glm::vec3 aPos = glm::vec3(a->transform[3].x, a->transform[3].y, a->transform[3].z);
		glm::vec3 bPos = glm::vec3(b->transform[3].x, b->transform[3].y, b->transform[3].z);

		return (glm::length(aPos - camPos) < glm::length(bPos - camPos));
	}

	glm::mat4 m;
	glm::vec3 camPos;
};

void drawSceneGeometry(vector<Model*> toDraw, const glm::mat4& proj, const glm::mat4& view){

	// Sort our models to make use of depth buffer
	std::sort(toDraw.begin(), toDraw.end(), CompareDist(view));
	glBindVertexArray(modelsVAO);

	glm::mat4 I;
	totalTriangles = 0;
	for (size_t i = 0; i < toDraw.size(); i++){
		//printf("%s - %d\n",toDraw[i]->name.c_str(),i);
		int lodIndex = 0; 
		assignLod(*toDraw[i], proj, view, &lodIndex);
		drawGeometry(*toDraw[i], -1, I, proj * view, lodIndex);
	}
}

//-------------  Final Composite --------------

Shader compositeShader;

unsigned int quadVAO;


void initFinalCompositeShader(){
	compositeShader = Shader("shaders/quad-vert.glsl", "shaders/quad-frag.glsl");
	compositeShader.init();
}

void drawCompositeImage(bool useBloom){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(quadVAO);
	glClear(GL_DEPTH_BUFFER_BIT);

	float bloomLevel = 0;
	compositeShader.bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brightText); 
	glUniform1i(glGetUniformLocation(compositeShader.ID, "texBright"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseTex);  //baseTex  depthMap
	glUniform1i(glGetUniformLocation(compositeShader.ID, "texDim"), 1);

	if (useBloom){  
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]); //pass in blured texture
		bloomLevel = 1;
	}

	glUniform1f(glGetUniformLocation(compositeShader.ID, "bloomAmount"), bloomLevel);

	glDrawArrays(GL_TRIANGLES, 0, 6);  //Draw Quad for final render
}

// --------- Fullscreen quad
void createFullscreenQuad(){
	float quad[24] = {1,1,1,1, -1,1,0,1, -1,-1,0,0,  1,-1,1,0, 1,1,1,1, -1,-1,0,0,};

  //Build a Vertex Array Object for the full screen Quad. This stores the VBO to shader attribute mappings
	glGenVertexArrays(1, &quadVAO); //Create a VAO
	glBindVertexArray(quadVAO); //Bind the above created VAO to the current context

	GLuint quadVBO;
  glGenBuffers(1, &quadVBO);  //Create 1 buffer called vbo
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO); //(Only one buffer can be bound at a time) 

	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  
  GLint quadPosAttrib = glGetAttribLocation(compositeShader.ID, "position");
  glVertexAttribPointer(quadPosAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
  //(above params: Attribute, vals/attrib., type, normalized?, stride, offset)
  glEnableVertexAttribArray(quadPosAttrib);

	GLint quadTexAttrib = glGetAttribLocation(compositeShader.ID, "texcoord");
	glVertexAttribPointer(quadTexAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(quadTexAttrib); 

  glBindVertexArray(0); //Unbind the VAO once we have set all the attributes
}

void cleanupBuffers(){
	glDeleteBuffers(1,&modelsVBO);
  glDeleteVertexArrays(1, &modelsVAO);
	glDeleteVertexArrays(1, &colliderVAO);
	//TODO: Clearn up the other VAOs and VBOs
}
