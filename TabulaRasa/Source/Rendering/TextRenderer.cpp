#include "TextRenderer.h"

#include <algorithm>

#include "../Platform/Platform.h"
#include "GL_shader.h"
#include "../Engine/Core/Memory.h"

#ifdef _WIN32
#include <intrin.h>
#endif

// Init the static shader to be NULL
GLShaderProgram* TextRenderer::g_TextRenderShader = NULL;
GLShaderProgram* TextRenderer::g_RectRenderShader = NULL;

// Init the static projection matrix to be the size of the screen
glm::mat4 TextRenderer::g_TextRenderProjectionMatrix = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f); //TODO: Update this and be watchful of ints (use floats)

// Init the RenderObjects
std::vector<TextRenderData2D*> TextRenderer::g_TextRenderObjects[NUM_LAYERS];
std::vector<RectRenderData2D*> TextRenderer::g_RectRenderObjects[NUM_LAYERS];

MemoryPool<TextRenderData2D>* TextRenderer::g_TextRenderDataMemoryPool = NULL;
MemoryPool<RectRenderData2D>* TextRenderer::g_RectRenderDataMemoryPool = NULL;

extern GameMemoryManager* g_MemoryManager;

bool SortRenderObjectsText(TextRenderData2D* FirstObject, TextRenderData2D* SecondObject)
{
	return FirstObject->Layer < SecondObject->Layer;
}

bool SortRenderObjectsRect(RectRenderData2D* FirstObject, RectRenderData2D* SecondObject)
{
	return FirstObject->Layer < SecondObject->Layer;
}

void TextRenderer::Initialize2DTextRendering()
{
	g_TextRenderDataMemoryPool = new MemoryPool<TextRenderData2D>(
		Allocate<TextRenderData2D>(g_MemoryManager->m_pRenderingMemory, 256),
		sizeof(TextRenderData2D) * 256);
	g_RectRenderDataMemoryPool = new MemoryPool<RectRenderData2D>(
		Allocate<RectRenderData2D>(g_MemoryManager->m_pRenderingMemory, 64),
		sizeof(RectRenderData2D) * 64);
	// Init the RenderObjects to hold 64 text objects by default (since it's a vector
	// it will grow automatically if it needs to
	for (unsigned int LayerID = 0; LayerID < NUM_LAYERS; ++LayerID)
	{
		g_TextRenderObjects[LayerID].reserve(32);
		g_RectRenderObjects[LayerID].reserve(32);
	}
}

void TextRenderer::Destroy2DTextRendering()
{
	for (unsigned int LayerID = 0; LayerID < NUM_LAYERS; ++LayerID)
	{
		for (auto& It : g_TextRenderObjects[LayerID])
		{
			g_TextRenderDataMemoryPool->DeallocateDelete(It);
		}
		g_TextRenderObjects[LayerID].clear();

		for (auto& It : g_RectRenderObjects[LayerID])
		{
			g_RectRenderDataMemoryPool->DeallocateDelete(It);
		}
		g_RectRenderObjects[LayerID].clear();
	}
}

struct GlyphVertex
{
	glm::vec3 Pos;
	glm::vec2 Tex;
};

TextRenderData2D* TextRenderer::AddTextToRender(const char* Text, const float& X, const float& Y, float Size, unsigned int Layer, unsigned int RenderFont)
{
	if (g_TextRenderShader == NULL)
		g_TextRenderShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("vertex_font_render.glsl"), std::string("fragment_font_render.glsl"));
	
	Font* FontToUse = GetLoadedFont(RenderFont);
	if (FontToUse == (Font*) 0xdddddddd) //TODO: Remove this
		return NULL;

	TextRenderData2D* NewTextRenderObject = g_TextRenderDataMemoryPool->Allocate();
	memset(NewTextRenderObject, NULL, sizeof(TextRenderData2D));

	glGenVertexArrays(1, &NewTextRenderObject->VAO);
	glBindVertexArray(NewTextRenderObject->VAO);

	glGenBuffers(1, &NewTextRenderObject->VBO);
	glGenBuffers(1, &NewTextRenderObject->IBO);

	NewTextRenderObject->Position = glm::vec3(X, Y, 0.0f);

	unsigned int Width = (unsigned int) FontToUse->SizeX;
	unsigned int Height = (unsigned int) FontToUse->SizeY;
	NewTextRenderObject->TextureID = FontToUse->Texture;

	unsigned int TextLength = strlen(Text);
	GlyphVertex* Vertices = AllocateTransient<GlyphVertex>(TextLength * 4);
	unsigned short* Indices = AllocateTransient<unsigned short>(TextLength * 6);
	unsigned int i = 0;
	float CharacterOffsetX = 0.0f;
	float CharacterOffsetY = 0.0f;
	for (i = 0; i < TextLength; ++i)
	{
		if (Text[i] == ' ')
		{
			// Advance with half the size
			CharacterOffsetX += Size / 2.0f;
			continue;
		}

		if (Text[i] == '\n')
		{
			CharacterOffsetX = 0.0f; // Reset the line width
			CharacterOffsetY += Size;
		}

		Glyph* CharacterGlyph = GetGlyphFromChar(Text[i], FontToUse);

		if (CharacterGlyph == NULL)
		{
			CharacterOffsetX += Size / 2.0f;
			continue;
		}

		unsigned int VertexOffset = i * 4; // the number of vertices offset from the previous set
		unsigned int IndexOffset = i * 6;

		// Calculate the scale of the character with this scale and font
		float ScaledWidth = Size * CharacterGlyph->Width / (float) FontToUse->Base;
		float ScaledHeight = Size * CharacterGlyph->Height / (float) FontToUse->Base;
		float BaseWidth = CharacterOffsetX + (Size * CharacterGlyph->XOffset / (float) FontToUse->LineHeight);
		float BaseHeight = CharacterOffsetY + (Size * CharacterGlyph->YOffset / (float) FontToUse->LineHeight);

		Vertices[VertexOffset + 0] = { 
			glm::vec3(BaseWidth,													BaseHeight,														1.0f),
			glm::vec2(CharacterGlyph->PositionX,									CharacterGlyph->PositionY) };
		Vertices[VertexOffset + 1] = { 
			glm::vec3(BaseWidth,													BaseHeight + ScaledHeight,										1.0f),
			glm::vec2(CharacterGlyph->PositionX,									CharacterGlyph->PositionY + CharacterGlyph->NormalizedHeight) };
		Vertices[VertexOffset + 2] = { 
			glm::vec3(BaseWidth + ScaledWidth,										BaseHeight + ScaledHeight,										1.0f),
			glm::vec2(CharacterGlyph->PositionX + CharacterGlyph->NormalizedWidth,	CharacterGlyph->PositionY + CharacterGlyph->NormalizedHeight) };
		Vertices[VertexOffset + 3] = { 
			glm::vec3(BaseWidth + ScaledWidth,										BaseHeight,														1.0f),
			glm::vec2(CharacterGlyph->PositionX + CharacterGlyph->NormalizedWidth,	CharacterGlyph->PositionY) };

		// Draw them in reverse order since the image is flipped (y=0 is not in the
		// lower part of the screen - like OpenGL has it - it is in the top of the
		// screen - like the monitor has it
		Indices[IndexOffset + 0] = VertexOffset + 3;
		Indices[IndexOffset + 1] = VertexOffset + 2;
		Indices[IndexOffset + 2] = VertexOffset + 0;
		Indices[IndexOffset + 3] = VertexOffset + 2;
		Indices[IndexOffset + 4] = VertexOffset + 1;
		Indices[IndexOffset + 5] = VertexOffset + 0;

		CharacterOffsetX += ScaledWidth;
	}

	glBindBuffer(GL_ARRAY_BUFFER, NewTextRenderObject->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GlyphVertex) * 4 * TextLength, Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewTextRenderObject->IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 6 * TextLength, Indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // Vertex position
	glEnableVertexAttribArray(1); // Vertex texture coordinate

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void*) offsetof(GlyphVertex, Pos));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void*) offsetof(GlyphVertex, Tex));

	// 6 vertices per glyph
	NewTextRenderObject->VertexCount = TextLength * 6;
	NewTextRenderObject->Layer = Layer;
	g_TextRenderObjects[Layer].push_back(NewTextRenderObject);
	glBindVertexArray(0);

	return NewTextRenderObject;
}

void TextRenderer::RemoveText(TextRenderData2D* TextToRemove)
{
	auto Position = std::find(g_TextRenderObjects[TextToRemove->Layer].begin(), g_TextRenderObjects[TextToRemove->Layer].end(), TextToRemove);
	if (Position != g_TextRenderObjects[TextToRemove->Layer].end())
	{
		TextRenderData2D* RenderData = *Position;
		glDeleteBuffers(1, &RenderData->VBO);
		glDeleteBuffers(1, &RenderData->IBO);
		g_TextRenderObjects[TextToRemove->Layer].erase(Position);

		g_TextRenderDataMemoryPool->Deallocate(RenderData);
	}
}

RectRenderData2D * TextRenderer::AddRectToRender(float MinX, float MinY, float MaxX, float MaxY, glm::vec4 Color, unsigned int Layer)
{
	if (g_RectRenderShader == NULL)
		g_RectRenderShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("vertex_rect_render.glsl"), std::string("fragment_rect_render.glsl"));

	RectRenderData2D* RenderData = g_RectRenderDataMemoryPool->Allocate();
	memset(RenderData, NULL, sizeof(RectRenderData2D));

	glGenVertexArrays(1, &RenderData->VAO);
	glBindVertexArray(RenderData->VAO);

	glGenBuffers(1, &RenderData->VBO);

	float Vertices[24] = { 
		MinX, MinY, Color.r, Color.g, Color.b, Color.a,
		MinX, MaxY, Color.r, Color.g, Color.b, Color.a,
		MaxX, MaxY, Color.r, Color.g, Color.b, Color.a,
		MaxX, MinY, Color.r, Color.g, Color.b, Color.a };

	glBindBuffer(GL_ARRAY_BUFFER, RenderData->VBO);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), Vertices, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*) (sizeof(float) * 2));

	glBindVertexArray(0);

	RenderData->Layer = Layer;
	g_RectRenderObjects[Layer].push_back(RenderData);
	return RenderData;
}

void TextRenderer::RemoveRect(RectRenderData2D* RectToRemove)
{
	auto Position = std::find(g_RectRenderObjects[RectToRemove->Layer].begin(), g_RectRenderObjects[RectToRemove->Layer].end(), RectToRemove);
	if (Position != g_RectRenderObjects[RectToRemove->Layer].end())
	{
		RectRenderData2D* RenderData = *Position;
		glDeleteBuffers(1, &RenderData->VBO);
		g_RectRenderObjects[RectToRemove->Layer].erase(Position);

		// This needs to be done since std::vector does not automatically
		// destruct the object (if it's a pointer, which it is) when calling erase
		g_RectRenderDataMemoryPool->Deallocate(RenderData);
	}
}

void TextRenderer::Render()
{
	glEnable(GL_BLEND);

	for (unsigned int LayerID = 0; LayerID < NUM_LAYERS; ++LayerID)
	{
		if (g_RectRenderShader)
		{
			g_RectRenderShader->Bind();
			g_RectRenderShader->SetProjectionMatrix(g_TextRenderProjectionMatrix);

			for (auto& It : g_RectRenderObjects[LayerID])
			{
				if (!It)
					continue;

				static GLubyte QUAD_INDICES[6] = { 3, 2, 0, 2, 1, 0 };

				glBindVertexArray(It->VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, QUAD_INDICES);
			}
		}

		if (g_TextRenderShader)
		{
			g_TextRenderShader->Bind();
			g_TextRenderShader->SetProjectionMatrix(g_TextRenderProjectionMatrix); //TODO: Do we need to update this every frame?

			for (auto& It : g_TextRenderObjects[LayerID])
			{
				if (!It)
					continue;

				g_TextRenderShader->SetPositionOffset(It->Position);

				glBindVertexArray(It->VAO);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, It->TextureID);

				glDrawElements(GL_TRIANGLES, It->VertexCount, GL_UNSIGNED_SHORT, (void*)0);
			}
		}
	}

	glBindVertexArray(0);
	glDisable(GL_BLEND);
}
