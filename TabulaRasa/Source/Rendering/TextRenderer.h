#pragma once

#include <string>

#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "Fonts.h"

#define PRINT_DEBUG_STRING(str, x, y) \
static TextRenderData2D* DebugText; \
if (DebugText)\
	TextRenderer::RemoveText(DebugText);\
DebugText = TextRenderer::AddTextToRender(Buffer, x, y);\

class GLShaderProgram;

struct TextRenderData2D
{
	/**
	 * Stores all the vertex attribute pointers state
	 */
	GLuint VAO;

	/**
	 * Vertex buffer object containing the vertex position data
	 * gathered from the font .fnt file
	 */
	GLuint VBO;

	/**
	 * Index buffer object containing the vertex data for
	 * rendering the font
	 */
	GLuint IBO;

	//TODO: move this into separate font class (with library)
	GLuint TextureID;

	/**
	 * Screen coord�nate to render to
	 */
	glm::vec3 Position;

	/**
	 * Number of vertices to render (amount of indices really)
	 */
	GLuint VertexCount;
};

class TextRenderer
{
public:

	static void Initialize2DTextRendering();

	static void Destroy2DTextRendering();

	static TextRenderData2D* AddTextToRender(const char* Text, const float& X = 0.0f, const float& Y = 0.0f, float Size = 24.0f, unsigned int Font = 0);

	static void RemoveText(TextRenderData2D* TextToRemove);

	static void Render();

	static std::vector<TextRenderData2D*> RenderObjects;

private:

	static GLShaderProgram* TextRenderShader;

	static glm::mat4 TextRenderProjectionMatrix;
};