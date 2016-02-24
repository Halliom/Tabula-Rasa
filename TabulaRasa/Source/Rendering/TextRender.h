#pragma once

#include <string>

#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "Fonts.h"

class GLShaderProgram;

class TextRender
{
public:
	TextRender();

	~TextRender();
	
	void SetTextToRender(const char* Text, float Size = 100, unsigned int Font = 0);

	void Render();

private:

	static GLShaderProgram* Shader;

	char* TextToRender;

	//ADD PROJECTION MATRIX WHICH IS AN ORTHOGONAL PROJECTION AND ADD
	//SIZE PARAMETER TO SETTEXTTORENDER

	glm::mat4 ProjectionMatrix;

	GLuint VBO;

	GLuint IBO;

	GLuint VAO;

	//TODO: move this into separate font class (with library)
	GLuint TextureID;

	unsigned int NumberOfVertices;
};