#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

#define FONT_DEFAULT 0
#define FONT_LIBRARY_SIZE 1

struct Glyph
{
	unsigned int GlyphID;

	float PositionX;
	float PositionY;

	float NormalizedWidth;
	float NormalizedHeight;

	float Width;
	float Height;
};

struct Font
{
	std::unordered_map<unsigned int, Glyph*> FontGlyphs;
	unsigned int GlyphCount;

	float SizeX;
	float SizeY;

	char* TextureAtlasFileName;
	std::vector<unsigned char>* FontImage;
};

static Font** LoadedFonts;

Font* GetLoadedFont(unsigned int Index);

Glyph* GetGlyphFromChar(const char& Char, const Font* FontToSearch);

void LoadFontLibrary(std::string* FontLibraryLocation);

Font* LoadFontFromFile(const char* FontFileName, const char* Directory);

void UnloadFontLibrary();