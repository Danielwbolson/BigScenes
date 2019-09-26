#include "GPU-Includes.h"
#include "Shader.h"
#include <external/loguru.hpp>
#include "Bloom.h"
#include "RenderingCore.h"

bool horizontal;
unsigned int numBloomPasses = 4;


Shader blurShader;


void initBloom(){
	blurShader = Shader("shaders/blur-vert.glsl", "shaders/blur-frag.glsl"); 
	blurShader.init();
}

//TODO: Make bloom faster/better, e.g., pingponging into smaller textures
void computeBloomBlur(){
	glBindVertexArray(quadVAO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	blurShader.bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brightText); 
	glUniform1i(glGetUniformLocation(blurShader.ID, "image"), 0);

	horizontal = true;
	bool first_iteration = true;
	for (unsigned int i = 0; i < numBloomPasses; i++){
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		glUniform1i(glGetUniformLocation(blurShader.ID, "horizontalBlur"), horizontal);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? brightText : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
		glDrawArrays(GL_TRIANGLES, 0, 6);  
		horizontal = !horizontal;
		if (first_iteration)
				first_iteration = false;
	}
}