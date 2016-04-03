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

#define NUM_LAYERS 4

template<typename T>
struct MemoryPool;

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
	 * Screen coordínate to render to
	 */
	glm::vec3 Position;

	/**
	 * Number of vertices to render (amount of indices really)
	 */
	GLuint VertexCount;

	/** 
	 * The layer this is on, higher layer means it gets
	 * rendered later
	 */
	unsigned int Layer;
};

struct RectRenderData2D
{
	/**
	* Stores all the vertex attribute pointers state
	*/
	GLuint VAO;

	/**
	* Vertex buffer object containing the vertex position data
	* of the rect
	*/
	GLuint VBO;

	/**
	* Screen coordínate to render to
	*/
	glm::vec3 Position;

	/**
	* The layer this is on, higher layer means it gets
	* rendered later
	*/
	unsigned int Layer;
};

class TextRenderer
{
public:

	static void Initialize2DTextRendering();

	static void Destroy2DTextRendering();

	static TextRenderData2D* AddTextToRender(const char* Text, const float& X = 0.0f, const float& Y = 0.0f, float Size = 24.0f, unsigned int Layer = 0, unsigned int Font = 0);

	static void RemoveText(TextRenderData2D* TextToRemove);

	static RectRenderData2D* AddRectToRender(float MinX, float MinY, float MaxX, float MaxY, glm::vec4 Color, unsigned int Layer = 0);

	static void RemoveRect(RectRenderData2D* RectToRemove);

	static void Render();

	static std::vector<TextRenderData2D*> g_TextRenderObjects[NUM_LAYERS];

	static std::vector<RectRenderData2D*> g_RectRenderObjects[NUM_LAYERS];

private:

	static GLShaderProgram* g_TextRenderShader;

	static GLShaderProgram* g_RectRenderShader;

	static glm::mat4 g_TextRenderProjectionMatrix;

	static MemoryPool<TextRenderData2D>* g_TextRenderDataMemoryPool;

	static MemoryPool<RectRenderData2D>* g_RectRenderDataMemoryPool;
};