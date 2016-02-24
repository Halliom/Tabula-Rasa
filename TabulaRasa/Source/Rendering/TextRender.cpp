#include "TextRender.h"

#include "../Platform/Platform.h"
#include "GL_shader.h"

#ifdef _WIN32
#include <intrin.h>
#endif

// Init the static shader to be NULL
GLShaderProgram* TextRender::Shader = NULL;

TextRender::TextRender()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO); //TODO: Merge these into the same glGenBuffers() call?
	glGenBuffers(1, &IBO);
	glGenTextures(1, &TextureID);
}

TextRender::~TextRender()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteTextures(1, &TextureID);
}

struct GlyphVertex
{
	glm::vec3 Pos;
	glm::vec2 Tex;
};

void TextRender::SetTextToRender(const char* Text, float Size, unsigned int RenderFont)
{
	uint64_t Cycles_Beginning = __rdtsc();
	ProjectionMatrix = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f); //TODO: Update this and be watchful of ints (use floats)
	uint64_t Cycles_ProjectionMatrix = __rdtsc();

	glBindVertexArray(VAO);
	if (Shader == NULL)
		Shader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("vertex_font_render.glsl"), std::string("fragment_font_render.glsl"));
	uint64_t Cycles_ShaderCreation = __rdtsc();

	TextToRender = (char*) Text;

	Font* FontToUse = GetLoadedFont(RenderFont);
	if (FontToUse == (Font*) 0xdddddddd) //TODO: Remove this
		return;

	unsigned int Width = 0;
	unsigned int Height = 0;
	std::vector<unsigned char>* FontImage = FontToUse->FontImage;

	uint64_t Cycles_ImageLoading = __rdtsc();

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
		Vertices[VertexOffset + 0] = { glm::vec3(CharacterOffset,			0.0f, 1.0f),
			glm::vec2(CharacterGlyph->PositionX, CharacterGlyph->PositionY) };
		Vertices[VertexOffset + 1] = { glm::vec3(CharacterOffset,			Size, 1.0f),
			glm::vec2(CharacterGlyph->PositionX, CharacterGlyph->PositionY + CharacterGlyph->Height) };
		Vertices[VertexOffset + 2] = { glm::vec3(CharacterOffset + Size,	Size, 1.0f),
			glm::vec2(CharacterGlyph->PositionX + CharacterGlyph->Width, CharacterGlyph->PositionY + CharacterGlyph->Height) };
		Vertices[VertexOffset + 3] = { glm::vec3(CharacterOffset + Size,	0.0f, 1.0f),
			glm::vec2(CharacterGlyph->PositionX + CharacterGlyph->Width, CharacterGlyph->PositionY) };

		// Draw them in reverse order since the image is flipped (y=0 is not in the
		// lower part of the screen - like OpenGL has it - it is in the top of the
		// screen - like the monitor has it
		Indices[IndexOffset + 0] = VertexOffset + 3;
		Indices[IndexOffset + 1] = VertexOffset + 2;
		Indices[IndexOffset + 2] = VertexOffset + 0;
		Indices[IndexOffset + 3] = VertexOffset + 2;
		Indices[IndexOffset + 4] = VertexOffset + 1;
		Indices[IndexOffset + 5] = VertexOffset + 0;

		CharacterOffset += Size;
	}

	uint64_t Cycles_VertexCreation = __rdtsc();

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GlyphVertex) * 4 * TextLength, Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 6 * TextLength, Indices, GL_STATIC_DRAW);

	glBindTexture(GL_TEXTURE_2D, TextureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, FontImage);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	uint64_t Cycles_SendingToGPU = __rdtsc();

	delete[] Vertices;
	delete[] Indices;

	uint64_t Cycles_Cleanup = __rdtsc(); //270000 cycles on average

	NumberOfVertices = TextLength;
}

void TextRender::Render()
{
	glBindVertexArray(VAO);

	Shader->Bind();
	Shader->SetProjectionMatrix(ProjectionMatrix); //TODO: Do we need to update this every frame?

	glBindTexture(GL_TEXTURE_2D, TextureID);
	glActiveTexture(GL_TEXTURE0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void*) offsetof(GlyphVertex, Pos));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void*) offsetof(GlyphVertex, Tex));

	glDrawElements(GL_TRIANGLES, NumberOfVertices * 6, GL_UNSIGNED_SHORT, (void*) 0);
}
