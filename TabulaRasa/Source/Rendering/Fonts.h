#pragma once

#include <string>

#include "GL\glew.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "DynamicArray.cpp"

struct TrueTypeGlyph
{
	float TexCoordBottomX;
	float TexCoordBottomY;

	float TexCoordTopX;
	float TexCoordTopY;

	float Advance;

	float Width;
	float Height;

	float BearingX;
	float BearingY;
};

struct TrueTypeFont
{
	TrueTypeGlyph Glyphs[128];

	unsigned int Width;
	unsigned int Height;

	unsigned int Size;

	GLuint TextureObject;
};

class FontLibrary
{
public:

	void Initialize(std::string& FontLibraryLocation);

	void Destroy();

	TrueTypeFont LoadFontFromFile(char* FontFileName, int Size);

	TrueTypeFont GetFont(int Index);

	static FontLibrary* g_FontLibrary;
	
	DynamicArray<TrueTypeFont> m_LoadedFonts;

private:

	std::string m_FontLibraryLocation;

	FT_Library m_pFreeTypeLibrary;

};