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
glm::mat4 TextRenderer::g_TextRenderProjectionMatrix = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f); //TODO: Update this and be watchful of ints (use floats)

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
	g_TextRenderShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("vertex_font_render.glsl"), std::string("fragment_font_render.glsl"));

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

TextRenderData2D* TextRenderer::AddTextToRenderWithColor(const char* Text, const float& X, const float& Y, glm::vec4& Color, unsigned int Layer, unsigned int Font)
{
	TextRenderData2D* NewTextRenderObject = g_TextRenderDataMemoryPool->Allocate();
	memset(NewTextRenderObject, NULL, sizeof(TextRenderData2D));

	TrueTypeFont FontToUse = FontLibrary::g_FontLibrary->GetFont(Font);

	glGenVertexArrays(1, &NewTextRenderObject->VAO);
	glBindVertexArray(NewTextRenderObject->VAO);

	glGenBuffers(1, &NewTextRenderObject->VBO);
	glGenBuffers(1, &NewTextRenderObject->IBO);

	int StringLength = strlen(Text);

	float* Vertices = AllocateTransient<float>(20 * StringLength);
	unsigned short* Indices = AllocateTransient<unsigned short>(6 * StringLength);

	float OffsetX = 0.0f;
	float OffsetY = FontToUse.Size;
	for (unsigned int CurrentChar = 0; CurrentChar < StringLength; ++CurrentChar)
	{
		if (Text[CurrentChar] == '\n')
		{
			OffsetY += FontToUse.Size;
			OffsetX = 0.0f;
			continue;
		}

		TrueTypeGlyph CurrentGlyph = FontToUse.Glyphs[Text[CurrentChar]];
		float BaseX = OffsetX + CurrentGlyph.BearingX;
		float BaseY = OffsetY + (CurrentGlyph.Height - CurrentGlyph.BearingY);

		int BaseIndex = CurrentChar * 20;

		// (0, 0)
		Vertices[BaseIndex] =		BaseX;
		Vertices[BaseIndex + 1] =	BaseY;
		Vertices[BaseIndex + 2] =	1.0f;
		Vertices[BaseIndex + 3] =	CurrentGlyph.TexCoordBottomX;
		Vertices[BaseIndex + 4] =	CurrentGlyph.TexCoordTopY;

		// (0, 1)
		Vertices[BaseIndex + 5] =	BaseX;
		Vertices[BaseIndex + 6] =	BaseY - CurrentGlyph.Height;
		Vertices[BaseIndex + 7] =	1.0f;
		Vertices[BaseIndex + 8] =	CurrentGlyph.TexCoordBottomX;
		Vertices[BaseIndex + 9] =	CurrentGlyph.TexCoordBottomY;
		
		// (1, 1)
		Vertices[BaseIndex + 10] =	BaseX + CurrentGlyph.Width;
		Vertices[BaseIndex + 11] =	BaseY - CurrentGlyph.Height;
		Vertices[BaseIndex + 12] =	1.0f;
		Vertices[BaseIndex + 13] =	CurrentGlyph.TexCoordTopX;
		Vertices[BaseIndex + 14] =	CurrentGlyph.TexCoordBottomY;
		
		// (1, 0)
		Vertices[BaseIndex + 15] =	BaseX + CurrentGlyph.Width;
		Vertices[BaseIndex + 16] =	BaseY;
		Vertices[BaseIndex + 17] =	1.0f;
		Vertices[BaseIndex + 18] =	CurrentGlyph.TexCoordTopX;
		Vertices[BaseIndex + 19] =	CurrentGlyph.TexCoordTopY;

		int IndicesBaseIndex = CurrentChar * 6;
		int IndexBase = CurrentChar * 4; // What index we currently are at
		Indices[IndicesBaseIndex] =		IndexBase + 0; //3
		Indices[IndicesBaseIndex + 1] = IndexBase + 1; //2
		Indices[IndicesBaseIndex + 2] = IndexBase + 2; //0
		Indices[IndicesBaseIndex + 3] = IndexBase + 0; //2
		Indices[IndicesBaseIndex + 4] = IndexBase + 2; //1
		Indices[IndicesBaseIndex + 5] = IndexBase + 3; //0

		OffsetX += CurrentGlyph.Advance;
	}

	NewTextRenderObject->Position = glm::vec3(X, Y, 0.0f);

	glBindBuffer(GL_ARRAY_BUFFER, NewTextRenderObject->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20 * StringLength, Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewTextRenderObject->IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 6 * StringLength, Indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // Vertex position
	glEnableVertexAttribArray(1); // Vertex texture coordinate

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) (sizeof(float) * 3));

	// 6 vertices per glyph
	NewTextRenderObject->VertexCount = 6 * StringLength;
	NewTextRenderObject->Layer = Layer;
	NewTextRenderObject->TextureID = FontToUse.TextureObject;
	NewTextRenderObject->Color = Color;
	g_TextRenderObjects[Layer].push_back(NewTextRenderObject);
	glBindVertexArray(0);

	return NewTextRenderObject;
}

void TextRenderer::RemoveText(TextRenderData2D* TextToRemove)
{
	if (TextToRemove == NULL)
		return;

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
	if (RectToRemove == NULL)
		return;

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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
				g_TextRenderShader->SetColor(It->Color);

				glBindVertexArray(It->VAO);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, It->TextureID);

				glDrawElements(GL_TRIANGLES, It->VertexCount, GL_UNSIGNED_SHORT, (void*) 0);
			}
		}
	}

	glBindVertexArray(0);
	glDisable(GL_BLEND);
}
