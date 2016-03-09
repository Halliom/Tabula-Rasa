#include "TextRenderer.h"

#include <algorithm>

#include "../Platform/Platform.h"
#include "GL_shader.h"

#ifdef _WIN32
#include <intrin.h>
#endif

// Init the static shader to be NULL
GLShaderProgram* TextRenderer::TextRenderShader = NULL;

// Init the static projection matrix to be the size of the screen
glm::mat4 TextRenderer::TextRenderProjectionMatrix = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f); //TODO: Update this and be watchful of ints (use floats)

// Init the RenderObjects
std::vector<TextRenderData2D*> TextRenderer::RenderObjects = std::vector<TextRenderData2D*>();

void TextRenderer::Initialize2DTextRendering()
{
	// Init the RenderObjects to hold 64 text objects by default (since it's a vector
	// it will grow automatically if it needs to
	RenderObjects.reserve(32);
}

void TextRenderer::Destroy2DTextRendering()
{
	for (auto& It : RenderObjects)
	{
		if (It)
			delete It;
	}
	RenderObjects.clear();
}

struct GlyphVertex
{
	glm::vec3 Pos;
	glm::vec2 Tex;
};

TextRenderData2D* TextRenderer::AddTextToRender(const char* Text, const float& X, const float& Y, float Size, unsigned int RenderFont)
{
	if (TextRenderShader == NULL)
		TextRenderShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("vertex_font_render.glsl"), std::string("fragment_font_render.glsl"));
	
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
	float CharacterOffset = 0;
	for (i = 0; i < TextLength; ++i)
	{
		Glyph* CharacterGlyph = GetGlyphFromChar(Text[i], FontToUse);
		unsigned int VertexOffset = i * 4; // the number of vertices offset from the previous set
		unsigned int IndexOffset = i * 6;
		Vertices[VertexOffset + 0] = { 
			glm::vec3(CharacterOffset,										0.0f,												1.0f),
			glm::vec2(CharacterGlyph->PositionX,							CharacterGlyph->PositionY) };
		Vertices[VertexOffset + 1] = { 
			glm::vec3(CharacterOffset,										(Size * CharacterGlyph->Height),					1.0f),
			glm::vec2(CharacterGlyph->PositionX,							CharacterGlyph->PositionY + CharacterGlyph->NormalizedHeight) };
		Vertices[VertexOffset + 2] = { 
			glm::vec3(CharacterOffset + (Size * CharacterGlyph->Width),		(Size * CharacterGlyph->Height),					1.0f),
			glm::vec2(CharacterGlyph->PositionX + CharacterGlyph->NormalizedWidth,	CharacterGlyph->PositionY + CharacterGlyph->NormalizedHeight) };
		Vertices[VertexOffset + 3] = { 
			glm::vec3(CharacterOffset + (Size * CharacterGlyph->Width),		0.0f,												1.0f),
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

		CharacterOffset += (Size * CharacterGlyph->Width);
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
	RenderObjects.push_back(NewTextRenderObject);
	glBindVertexArray(0);

	return NewTextRenderObject;
}

void TextRenderer::RemoveText(TextRenderData2D* TextToRemove)
{
	auto Position = std::find(RenderObjects.begin(), RenderObjects.end(), TextToRemove);
	if (Position != RenderObjects.end())
	{
		TextRenderData2D* RenderData = *Position;
		glDeleteBuffers(1, &RenderData->VBO);
		glDeleteBuffers(1, &RenderData->IBO);
		glDeleteTextures(1, &RenderData->TextureID);
		RenderObjects.erase(Position);

		// This needs to be done since std::vector does not automatically
		// destruct the object (if it's a pointer, which it is) when calling erase
		delete RenderData;
	}
}

void TextRenderer::Render()
{
	TextRenderShader->Bind();
	TextRenderShader->SetProjectionMatrix(TextRenderProjectionMatrix); //TODO: Do we need to update this every frame?

	for (auto& It : RenderObjects)
	{
		if (!It)
			continue;

		TextRenderShader->SetPositionOffset(It->Position);

		glBindVertexArray(It->VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, It->TextureID);

		glDrawElements(GL_TRIANGLES, It->VertexCount, GL_UNSIGNED_SHORT, (void*)0);
	}
}
