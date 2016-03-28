#include "TextRenderer.h"

#include <algorithm>

#include "../Platform/Platform.h"
#include "GL_shader.h"

#ifdef _WIN32
#include <intrin.h>
#endif

// Init the static shader to be NULL
GLShaderProgram* TextRenderer::g_TextRenderShader = NULL;
GLShaderProgram* TextRenderer::g_RectRenderShader = NULL;

// Init the static projection matrix to be the size of the screen
glm::mat4 TextRenderer::g_TextRenderProjectionMatrix = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f); //TODO: Update this and be watchful of ints (use floats)

// Init the RenderObjects
std::vector<TextRenderData2D*> TextRenderer::g_TextRenderObjects;
std::vector<RectRenderData2D*> TextRenderer::g_RectRenderObjects;

void TextRenderer::Initialize2DTextRendering()
{
	// Init the RenderObjects to hold 64 text objects by default (since it's a vector
	// it will grow automatically if it needs to
	g_TextRenderObjects.reserve(32);
	g_RectRenderObjects.reserve(32);
}

void TextRenderer::Destroy2DTextRendering()
{
	for (auto& It : g_TextRenderObjects)
	{
		if (It)
			delete It;
	}
	g_TextRenderObjects.clear();
	
	for (auto& It : g_RectRenderObjects)
	{
		if (It)
			delete It;
	}
	g_RectRenderObjects.clear();
}

struct GlyphVertex
{
	glm::vec3 Pos;
	glm::vec2 Tex;
};

TextRenderData2D* TextRenderer::AddTextToRender(const char* Text, const float& X, const float& Y, float Size, unsigned int RenderFont)
{
	if (g_TextRenderShader == NULL)
		g_TextRenderShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("vertex_font_render.glsl"), std::string("fragment_font_render.glsl"));
	
	Font* FontToUse = GetLoadedFont(RenderFont);
	if (FontToUse == (Font*) 0xdddddddd) //TODO: Remove this
		return NULL;

	TextRenderData2D* NewTextRenderObject = new TextRenderData2D();

	glGenVertexArrays(1, &NewTextRenderObject->VAO);
	glBindVertexArray(NewTextRenderObject->VAO);

	glGenBuffers(1, &NewTextRenderObject->VBO);
	glGenBuffers(1, &NewTextRenderObject->IBO);
	glGenTextures(1, &NewTextRenderObject->TextureID);

	NewTextRenderObject->Position = glm::vec3(X, Y, 0.0f);

	unsigned int Width = (unsigned int) FontToUse->SizeX;
	unsigned int Height = (unsigned int) FontToUse->SizeY;
	std::vector<unsigned char>* FontImage = FontToUse->FontImage;

	unsigned int TextLength = strlen(Text);
	GlyphVertex* Vertices = new GlyphVertex[TextLength * 4];
	unsigned short* Indices = new unsigned short[TextLength * 6];
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
		float BaseWidth = CharacterOffsetX + Size * CharacterGlyph->XOffset / FontToUse->LineHeight;
		float BaseHeight = CharacterOffsetY + Size * CharacterGlyph->YOffset / FontToUse->LineHeight;

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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void*)offsetof(GlyphVertex, Pos));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void*)offsetof(GlyphVertex, Tex));

	glBindTexture(GL_TEXTURE_2D, NewTextRenderObject->TextureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(FontImage->at(0)));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	delete[] Vertices;
	delete[] Indices;

	// 6 vertices per glyph
	NewTextRenderObject->VertexCount = TextLength * 6;
	g_TextRenderObjects.push_back(NewTextRenderObject);
	glBindVertexArray(0);

	return NewTextRenderObject;
}

void TextRenderer::RemoveText(TextRenderData2D* TextToRemove)
{
	auto Position = std::find(g_TextRenderObjects.begin(), g_TextRenderObjects.end(), TextToRemove);
	if (Position != g_TextRenderObjects.end())
	{
		TextRenderData2D* RenderData = *Position;
		glDeleteBuffers(1, &RenderData->VBO);
		glDeleteBuffers(1, &RenderData->IBO);
		glDeleteTextures(1, &RenderData->TextureID);
		g_TextRenderObjects.erase(Position);

		// This needs to be done since std::vector does not automatically
		// destruct the object (if it's a pointer, which it is) when calling erase
		delete RenderData;
	}
}

RectRenderData2D * TextRenderer::AddRectToRender(float MinX, float MinY, float MaxX, float MaxY, glm::vec4 Color)
{
	if (g_RectRenderShader == NULL)
		g_RectRenderShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("vertex_rect_render.glsl"), std::string("fragment_rect_render.glsl"));

	RectRenderData2D* RenderData = new RectRenderData2D();

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

	g_RectRenderObjects.push_back(RenderData);
	return RenderData;
}

void TextRenderer::RemoveRect(RectRenderData2D * RectToRemove)
{
	auto Position = std::find(g_RectRenderObjects.begin(), g_RectRenderObjects.end(), RectToRemove);
	if (Position != g_RectRenderObjects.end())
	{
		RectRenderData2D* RenderData = *Position;
		glDeleteBuffers(1, &RenderData->VBO);
		g_RectRenderObjects.erase(Position);

		// This needs to be done since std::vector does not automatically
		// destruct the object (if it's a pointer, which it is) when calling erase
		delete RenderData;
	}
}

void TextRenderer::Render()
{
	glEnable(GL_BLEND);

	if (g_RectRenderShader)
	{
		g_RectRenderShader->Bind();
		g_RectRenderShader->SetProjectionMatrix(g_TextRenderProjectionMatrix);

		for (auto& It : g_RectRenderObjects)
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

		for (auto& It : g_TextRenderObjects)
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

	glBindVertexArray(0);
	glDisable(GL_BLEND);
}
