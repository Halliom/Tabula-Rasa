#include "Fonts.h"

#include "../Engine/Engine.h"
#include "../Engine/Core/Memory.h"

static inline int NextPower2(int x)
{
	int val = 1;
	while (val < x) val <<= 1;
	return val;
}

FontLibrary::~FontLibrary()
{
    FT_Done_FreeType(m_pFreeTypeLibrary);
}

TrueTypeFont FontLibrary::LoadFontFromFile(const char* FontFileName, int Size)
{
	FT_Face FontFace;

	std::string FullFileName = std::string(m_FontLibraryLocation).append(FontFileName);
	int ErrorCode = FT_New_Face(m_pFreeTypeLibrary, FullFileName.c_str(), 0, &FontFace);
	if (ErrorCode)
	{
		// TODO: Errro logging
		assert("Failure loading font");
	}

	// TODO: Change this so that you can vary the font size
	FT_Set_Pixel_Sizes(FontFace, 0, Size);

	TrueTypeFont NewFont = {};

	int NumGlyphsPerX = 16;
	int NumGlyphsPerY = 8;

	int GlyphWidth = 0;
	int GlyphHeight = 0;
	for (char c = 0; c < 127; ++c)
	{
		FT_Load_Char(FontFace, c, FT_LOAD_RENDER);
		FT_GlyphSlot LoadedGlyph = FontFace->glyph;

		if (LoadedGlyph->bitmap.width > GlyphWidth)
			GlyphWidth = LoadedGlyph->bitmap.width;

		if (LoadedGlyph->bitmap.rows > GlyphHeight)
			GlyphHeight = LoadedGlyph->bitmap.rows;
	}

	int TextureWidth = NextPower2(NumGlyphsPerX * GlyphWidth);
	int TextureHeight = NextPower2(NumGlyphsPerY * GlyphHeight);

	NewFont.GlyphBitmap.Use();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	NewFont.GlyphBitmap.LoadFromBuffer((unsigned char*)NULL, TextureWidth, TextureHeight, GL_RGB8, GL_RGB);
	NewFont.GlyphBitmap.SetFilteringMode(GL_LINEAR);

	for (char c = 0; c < 127; ++c)
	{
		FT_Load_Char(FontFace, c, FT_LOAD_RENDER);
		FT_GlyphSlot LoadedChar = FontFace->glyph;

		int BitmapWidth = LoadedChar->bitmap.width;
		int BitmapHeight = LoadedChar->bitmap.rows;

		int PositionX = c % NumGlyphsPerX;
		int PositionY = c / NumGlyphsPerX;
		int TexPositionX = PositionX * GlyphWidth;
		int TexPositionY = PositionY * GlyphHeight;

		NewFont.Glyphs[c].TexCoordBottomX = (float) TexPositionX / (float) TextureWidth;
		NewFont.Glyphs[c].TexCoordBottomY = (float) TexPositionY / (float) TextureHeight;

		NewFont.Glyphs[c].TexCoordTopX = (float) (TexPositionX + BitmapWidth) / (float) TextureWidth;
		NewFont.Glyphs[c].TexCoordTopY = (float) (TexPositionY + BitmapHeight) / (float) TextureHeight;

		// This needs to be shifted by 6 since LoadedChar->advance is in 
		// 1/64th of a pixel and 2^6=64
		NewFont.Glyphs[c].Advance = LoadedChar->advance.x >> 6;

		NewFont.Glyphs[c].Width = BitmapWidth;
		NewFont.Glyphs[c].Height = BitmapHeight;

		NewFont.Glyphs[c].BearingX = LoadedChar->bitmap_left;
		NewFont.Glyphs[c].BearingY = LoadedChar->bitmap_top;

		char* Buffer = AllocateTransient<char>(BitmapWidth * BitmapHeight * 3);
		for (int i = 0; i < BitmapHeight; ++i)
		{
			for (int j = 0; j < BitmapHeight; ++j)
			{
				unsigned int BaseIndex = ((j * BitmapWidth) + i);
				Buffer[BaseIndex * 3]		= LoadedChar->bitmap.buffer[BaseIndex];
				Buffer[(BaseIndex * 3) + 1] = LoadedChar->bitmap.buffer[BaseIndex];
				Buffer[(BaseIndex * 3) + 2] = LoadedChar->bitmap.buffer[BaseIndex];
			}
		}

		glTexSubImage2D(
			GL_TEXTURE_2D, 
			0, 
			TexPositionX, 
			TexPositionY, 
			BitmapWidth,
			BitmapHeight,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			Buffer);
	}

	NewFont.Size = Size;
	// Reset
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	FT_Done_Face(FontFace);
	m_LoadedFonts.Push(NewFont);
	return NewFont;
}

TrueTypeFont FontLibrary::GetFont(int Index)
{
	return m_LoadedFonts[Index];
}

void FontLibrary::Initialize(std::string& FontLibraryLocation)
{
	if (FT_Init_FreeType(&m_pFreeTypeLibrary))
	{
		// TODO: Error logging
		// Freetype could not initialize
		assert(false);
	}

#ifdef _WIN32
	m_FontLibraryLocation = FontLibraryLocation.append("\\");
#else
    m_FontLibraryLocation = FontLibraryLocation.append("/");
#endif

	m_LoadedFonts = List<TrueTypeFont>(g_Engine->g_MemoryManager->m_pGameMemory);

#if 0
	HANDLE FileHandle;
	WIN32_FIND_DATAA FindData;

	if (FontLibraryLocation.at(0) == '\0')
		FileHandle = FindFirstFileA("*.*", &FindData); // search through all files in current dir
	else
		FileHandle = FindFirstFileA(FontLibraryLocation.append("\\*").c_str(), &FindData);

	if (FileHandle == INVALID_HANDLE_VALUE)
		return;

	unsigned int CurrentFontIndex = 0;
	do
	{
		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			continue;
		if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;
		char* FileExtension = strrchr(FindData.cFileName, '.');
		if (FileExtension != NULL && strcmp(FileExtension, ".ttf") == 0)
		{
			// Load the font from file
			FontLibraryLocation.pop_back(); //remove last character

											   // Save the directory
			char Directory[MAX_PATH];
			strcpy(Directory, FontLibraryLocation.c_str());

			TrueTypeFont LoadedFont = LoadFontFromFile((char*) FontLibraryLocation.append(FindData.cFileName).c_str(), Directory);
			m_LoadedFonts.Push(LoadedFont);
			FontLibraryLocation.assign(Directory);
		}
	} while (FindNextFileA(FileHandle, &FindData));
	FindClose(FileHandle);
#endif
}