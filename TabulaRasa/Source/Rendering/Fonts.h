#pragma once

#include <string>

#include "GL/gl3w.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../Engine/Core/Memory.h"
#include "../Engine/Core/List.h"

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
    
    ~FontLibrary();

	void Initialize(std::string& FontLibraryLocation);

	TrueTypeFont LoadFontFromFile(const char* FontFileName, int Size);

	TrueTypeFont GetFont(int Index);
	
	List<TrueTypeFont> m_LoadedFonts;

private:

	std::string m_FontLibraryLocation;

	FT_Library m_pFreeTypeLibrary;

};