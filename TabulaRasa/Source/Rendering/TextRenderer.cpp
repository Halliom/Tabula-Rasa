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
List<TextRenderData2D> TextRenderer::g_TextRenderObjects[NUM_LAYERS];
List<RectRenderData2D> TextRenderer::g_RectRenderObjects[NUM_LAYERS];

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
		g_TextRenderObjects[LayerID] = List<TextRenderData2D>(g_MemoryManager->m_pGameMemory);
		g_RectRenderObjects[LayerID] = List<RectRenderData2D>(g_MemoryManager->m_pGameMemory);

		g_TextRenderObjects[LayerID].Reserve(32);
		g_RectRenderObjects[LayerID].Reserve(32);
	}
}

void TextRenderer::Destroy2DTextRendering()
{
	for (unsigned int LayerID = 0; LayerID < NUM_LAYERS; ++LayerID)
	{
		for (int Index = 0; Index < g_TextRenderObjects[LayerID].Size; ++Index)
		{
			g_TextRenderDataMemoryPool->DeallocateDelete(&g_TextRenderObjects[LayerID][Index]);
		}
		g_TextRenderObjects[LayerID].Reserve(0);

		for (int Index = 0; Index < g_RectRenderObjects[LayerID].Size; ++Index)
		{
			g_RectRenderDataMemoryPool->DeallocateDelete(&g_RectRenderObjects[LayerID][Index]);
		}
		g_RectRenderObjects[LayerID].Reserve(0);
	}
}

struct GlyphVertex
{
	glm::vec3 Pos;
	glm::vec2 Tex;
};

TextRenderData2D* TextRenderer::AddTextToRenderWithColorAndLength(const char* Text, size_t StringLength, const float& X, const float& Y, glm::vec4& Color, unsigned int Layer, TrueTypeFont* Font)
{
	TextRenderData2D* NewTextRenderObject = g_TextRenderDataMemoryPool->Allocate();
	memset(NewTextRenderObject, NULL, sizeof(TextRenderData2D));

	TrueTypeFont FontToUse;
	if (Font == NULL)
	{
		FontToUse = FontLibrary::g_FontLibrary->GetFont(0);
	}
	else
	{
		FontToUse = *Font;
	}

	glGenVertexArrays(1, &NewTextRenderObject->VAO);
	glBindVertexArray(NewTextRenderObject->VAO);

	glGenBuffers(1, &NewTextRenderObject->VBO);
	glGenBuffers(1, &NewTextRenderObject->IBO);

	List<float> Vertices = List<float>(g_MemoryManager->m_pGameMemory);
	Vertices.Reserve(StringLength * 20);
	List<unsigned short> Indices = List<unsigned short>(g_MemoryManager->m_pGameMemory);
	Indices.Reserve(StringLength * 6);
	unsigned int NumGlyphs = 0;

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

		// What index (of vertex) we are at right now
		// There are 5 floats per vertex so to get the index
		// of the entire vertex we divide the total number of
		// floats by 5.
		int BaseIndex = Vertices.Size / 5;

		// (0, 0)
		Vertices.Push(BaseX);
		Vertices.Push(BaseY);
		Vertices.Push(1.0f);
		Vertices.Push(CurrentGlyph.TexCoordBottomX);
		Vertices.Push(CurrentGlyph.TexCoordTopY);

		// (0, 1)
		Vertices.Push(BaseX);
		Vertices.Push(BaseY - CurrentGlyph.Height);
		Vertices.Push(1.0f);
		Vertices.Push(CurrentGlyph.TexCoordBottomX);
		Vertices.Push(CurrentGlyph.TexCoordBottomY);
		
		// (1, 1)
		Vertices.Push(BaseX + CurrentGlyph.Width);
		Vertices.Push(BaseY - CurrentGlyph.Height);
		Vertices.Push(1.0f);
		Vertices.Push(CurrentGlyph.TexCoordTopX);
		Vertices.Push(CurrentGlyph.TexCoordBottomY);
		
		// (1, 0)
		Vertices.Push(BaseX + CurrentGlyph.Width);
		Vertices.Push(BaseY);
		Vertices.Push(1.0f);
		Vertices.Push(CurrentGlyph.TexCoordTopX);
		Vertices.Push(CurrentGlyph.TexCoordTopY);

		int IndexBase = CurrentChar * 4; // What index we currently are at
		Indices.Push(BaseIndex + 0); //3
		Indices.Push(BaseIndex + 1); //2
		Indices.Push(BaseIndex + 2); //0
		Indices.Push(BaseIndex + 0); //2
		Indices.Push(BaseIndex + 2); //1
		Indices.Push(BaseIndex + 3); //0

		OffsetX += CurrentGlyph.Advance;
		++NumGlyphs;
	}

	NewTextRenderObject->Position = glm::vec3(X, Y, 0.0f);

	glBindBuffer(GL_ARRAY_BUFFER, NewTextRenderObject->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Vertices.Size, Vertices.Data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewTextRenderObject->IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * Indices.Size, Indices.Data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // Vertex position
	glEnableVertexAttribArray(1); // Vertex texture coordinate

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) (sizeof(float) * 3));

	// 6 vertices per glyph
	NewTextRenderObject->VertexCount = Indices.Size;
	NewTextRenderObject->Layer = Layer;
	NewTextRenderObject->TextureID = FontToUse.TextureObject;
	NewTextRenderObject->Color = Color;
	g_TextRenderObjects[Layer].Push(*NewTextRenderObject);
	glBindVertexArray(0);

	return NewTextRenderObject;
}

void TextRenderer::RemoveText(TextRenderData2D* TextToRemove)
{
	if (TextToRemove == NULL)
		return;

	glDeleteBuffers(1, &TextToRemove->VBO);
	glDeleteBuffers(1, &TextToRemove->IBO);
	glDeleteVertexArrays(1, &TextToRemove->VAO);

	g_TextRenderObjects[TextToRemove->Layer].Remove(*TextToRemove);
	g_TextRenderDataMemoryPool->Deallocate(TextToRemove);
}

RectRenderData2D* TextRenderer::AddRectToRender(float MinX, float MinY, float MaxX, float MaxY, glm::vec4 Color, unsigned int Layer)
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
	g_RectRenderObjects[Layer].Push(*RenderData);
	return RenderData;
}

void TextRenderer::RemoveRect(RectRenderData2D* RectToRemove)
{
	if (RectToRemove == NULL)
		return;

	glDeleteBuffers(1, &RectToRemove->VBO);
	g_RectRenderObjects[RectToRemove->Layer].Remove(*RectToRemove);

	// This needs to be done since std::vector does not automatically
	// destruct the object (if it's a pointer, which it is) when calling erase
	g_RectRenderDataMemoryPool->Deallocate(RectToRemove);
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

			for (int i = 0; i < g_RectRenderObjects[LayerID].Size; ++i)
			{
				//if (!&g_RectRenderObjects[LayerID][i])
				//	continue;

				static GLubyte QUAD_INDICES[6] = { 3, 2, 0, 2, 1, 0 };

				glBindVertexArray(g_RectRenderObjects[LayerID][i].VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, QUAD_INDICES);
			}
		}

		if (g_TextRenderShader)
		{
			g_TextRenderShader->Bind();
			g_TextRenderShader->SetProjectionMatrix(g_TextRenderProjectionMatrix); //TODO: Do we need to update this every frame?

			for (int i = 0; i < g_TextRenderObjects[LayerID].Size; ++i)
			{
				//if (!It)
				//	continue;

				g_TextRenderShader->SetPositionOffset(g_TextRenderObjects[LayerID][i].Position);
				g_TextRenderShader->SetColor(g_TextRenderObjects[LayerID][i].Color);

				glBindVertexArray(g_TextRenderObjects[LayerID][i].VAO);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, g_TextRenderObjects[LayerID][i].TextureID);

				glDrawElements(GL_TRIANGLES, g_TextRenderObjects[LayerID][i].VertexCount, GL_UNSIGNED_SHORT, (void*) 0);
			}
		}
	}

	glBindVertexArray(0);
	glDisable(GL_BLEND);
}
