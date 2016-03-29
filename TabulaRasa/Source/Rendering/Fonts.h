#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

#include "GL\glew.h"

#define FONT_DEFAULT 0
#define FONT_LIBRARY_SIZE 1

struct Glyph
{
	unsigned int GlyphID;

	float PositionX;
	float PositionY;

	float NormalizedWidth;
	float NormalizedHeight;

	float XOffset;
	float YOffset;
	float XAdvance;

	float Width;
	float Height;
};

struct Font
{
	std::unordered_map<unsigned int, Glyph*> FontGlyphs;
	unsigned int GlyphCount;

	float SizeX;
	float SizeY;

	unsigned int LineHeight;
	unsigned int Base;

	char* TextureAtlasFileName;
	GLuint Texture;
};

static Font** LoadedFonts;

Font* GetLoadedFont(unsigned int Index);

Glyph* GetGlyphFromChar(const char& Char, const Font* FontToSearch);

void LoadFontLibrary(std::string* FontLibraryLocation);

Font* LoadFontFromFile(const char* FontFileName, const char* Directory);

void UnloadFontLibrary();