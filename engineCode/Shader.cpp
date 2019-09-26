#include "Shader.h"
#include "GPU-Includes.h"
#include <external/loguru.hpp>


// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile){
	FILE *fp;
	long length;
	char *buffer;

	// open the file containing the text of the shader code
	fp = fopen(shaderFile, "rb");

	// check for errors in opening the file
	if (fp == NULL) {
		printf("can't open shader source file %s", shaderFile);
		return NULL;
	}

	// determine the file size
	fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
	length = ftell(fp);  // return the value of the current position

	// allocate a buffer with the indicated number of bytes, plus one (for null termination)
	buffer = new char[length + 1];

	// read the appropriate number of bytes from the file
	fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
	fread(buffer, 1, length, fp); // read all of the bytes

	// append a NULL character to indicate the end of the string
	buffer[length] = '\0';

	// close the file
	fclose(fp);

	// return the string
	return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
void Shader::init(){
	GLuint vertex_shader, fragment_shader;
	GLchar *vs_text, *fs_text;
	GLuint program;

	// check GLSL version
	LOG_F(1,"Compiling against GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Create shader handlers
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read source code from shader files
	vs_text = readShaderSource(VertexShaderName.c_str());
	fs_text = readShaderSource(FragmentShaderName.c_str());

	CHECK_NOTNULL_F(vs_text,"Failed to read from vertex shader file %s", VertexShaderName.c_str());
	CHECK_NOTNULL_F(fs_text,"Failed to read from fragent shader file %s", FragmentShaderName.c_str());

	// Load Vertex Shader
	const char *vv = vs_text;
	glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
	glCompileShader(vertex_shader); // Compile shaders
	
	// Check for errors
	GLint  compiled;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		LOG_F(ERROR,"Vertex shader '%s' failed to compile:",VertexShaderName.c_str());
		GLint logMaxSize, logLength;
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
		char* logMsg = new char[logMaxSize];
		glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
		LOG_F(ERROR,"Shader error message: %s", logMsg);
		exit(1);
	}
	
	// Load Fragment Shader
	const char *ff = fs_text;
	glShaderSource(fragment_shader, 1, &ff, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	
	//Check for Errors
	if (!compiled) {
		LOG_F(ERROR,"Fragment shader '%s' failed to compile",FragmentShaderName.c_str());
		GLint logMaxSize, logLength;
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
		char* logMsg = new char[logMaxSize];
		glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
		LOG_F(ERROR,"Shader error message: %s", logMsg);
		exit(1);
	}

	// Create the program
	program = glCreateProgram();

	// Attach shaders to program
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	// Link and set program to use
	glLinkProgram(program);

	ID = program;
}

void Shader::bind(){
	glUseProgram(ID);
}

Shader::~Shader(){
	//printf("Deleted %s\n",VertexShaderName.c_str());
	if (ID >= 0) glDeleteProgram(ID);
}