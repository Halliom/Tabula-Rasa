#include "Fonts.h"

#include "windows.h"
#include "../Platform/Platform.h"

//TODO: Force inline?
inline int GetSequentialLineValue()
{
	char* Argument = strtok(NULL, " ");
	char* StartOfValue = strrchr(Argument, '=');
	char* EndOfValue = strrchr(StartOfValue + 1, '\0');
	char* Value = new char[EndOfValue - StartOfValue];

	strncpy(Value, StartOfValue + 1, EndOfValue - StartOfValue);
	Value[EndOfValue - StartOfValue] = '\0';

	return atoi(Value);
}

Font* GetLoadedFont(unsigned int Index)
{
	return LoadedFonts[min(max(Index, FONT_LIBRARY_SIZE - 1), 0)];
}

Glyph* GetGlyphFromChar(const char& Char, const Font* FontToSearch)
{
	const auto FindResult = FontToSearch->FontGlyphs.find((unsigned int) Char);
	if (FindResult == FontToSearch->FontGlyphs.end())
		return NULL;
	else
		return ((*FindResult).second);
}

void LoadFontLibrary(std::string* font_library_location)
{
	LoadedFonts = new Font*[FONT_LIBRARY_SIZE];

	HANDLE FileHandle;
	WIN32_FIND_DATAA FindData;

	if (font_library_location->at(0) == '\0') 
		FileHandle = FindFirstFileA("*.*", &FindData); // search through all files in current dir
	else
		FileHandle = FindFirstFileA(font_library_location->append("\\*").c_str(), &FindData);

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
		if (FileExtension != NULL && strcmp(FileExtension, ".fnt") == 0)
		{
			// Load the font from file
			font_library_location->pop_back(); //remove last character

			// Save the directory
			char Directory[MAX_PATH];
			strcpy(Directory, font_library_location->c_str());

			LoadedFonts[CurrentFontIndex] = LoadFontFromFile(font_library_location->append(FindData.cFileName).c_str(), Directory);
			font_library_location->assign(Directory);
		}
	} while (FindNextFileA(FileHandle, &FindData));
	FindClose(FileHandle);
}

Font* LoadFontFromFile(const char* FontFileName, const char* Directory)
{
	FILE* FilePointer = fopen(FontFileName, "r");

	char Line[150];

	Font* NewFont = new Font();

	unsigned int LineNumber = 0;
	while (fgets(Line, 150, FilePointer))
	{
		switch (LineNumber)
		{
			case 0: // first line is info
			{

				break;
			}
			case 1: // second line is common
			{
				char* Argument = strtok(Line, " "); // Skip the word "common"
				NewFont->LineHeight = GetSequentialLineValue();
				NewFont->Base = GetSequentialLineValue();
				NewFont->SizeX = (float) GetSequentialLineValue();
				NewFont->SizeY = (float) GetSequentialLineValue();
				break;
			}
			case 2: // third line is pages
			{
				char* Argument = strtok(Line, " ");
				Argument = strtok(NULL, " "); // skip the first two tokens
				Argument = strtok(NULL, " ");
				if (Argument)
				{
					char* FileName = strrchr(Argument, '=');
					char* EndOfFileName = strrchr(FileName + 2, '\"');
					unsigned int DirectoryLength = strlen(Directory);
					NewFont->TextureAtlasFileName = new char[EndOfFileName - FileName - 2 + DirectoryLength]; // +1 because we need to null terminate
					strcpy(NewFont->TextureAtlasFileName, Directory);
					strncpy(NewFont->TextureAtlasFileName + DirectoryLength, FileName + 2, EndOfFileName - FileName - 2);
					NewFont->TextureAtlasFileName[EndOfFileName - FileName - 2 + DirectoryLength] = '\0'; // Null-terminate
				}
				break;
			}
			case 3: // fourth line is charcount
			{
				char* Argument = strtok(Line, " ");
				Argument = strtok(NULL, " ");
				if (Argument)
				{
					char* StartOfValue = strrchr(Argument, '=');
					char* EndOfValue = strrchr(StartOfValue + 1, '\n');
					char* Value = new char[EndOfValue - StartOfValue];

					strncpy(Value,StartOfValue + 1, EndOfValue - StartOfValue);
					Value[EndOfValue - StartOfValue] = '\0';
					
					int CharacterCount = atoi(Value);
					NewFont->FontGlyphs.reserve(CharacterCount);
					NewFont->GlyphCount = CharacterCount;
				}
				break;
			}
			default: //rest are all character definitions
			{
				char* Argument = strtok(Line, " "); // Skip first value since it only says char
				Glyph* NewGlyph = new Glyph();
				NewGlyph->GlyphID = GetSequentialLineValue();
				NewGlyph->PositionX = (float) GetSequentialLineValue() / NewFont->SizeX;
				NewGlyph->PositionY = (float) GetSequentialLineValue() / NewFont->SizeY;
				NewGlyph->Width = (float) GetSequentialLineValue();
				NewGlyph->Height = (float) GetSequentialLineValue();
				NewGlyph->NormalizedWidth = (float) NewGlyph->Width / NewFont->SizeX;
				NewGlyph->NormalizedHeight = (float) NewGlyph->Height / NewFont->SizeY;
				NewGlyph->XOffset = (float) GetSequentialLineValue();
				NewGlyph->YOffset = (float) GetSequentialLineValue();
				NewGlyph->XAdvance = (float) GetSequentialLineValue();

				NewFont->FontGlyphs.insert({ NewGlyph->GlyphID, NewGlyph });
			}
		}
		++LineNumber;
	}

	unsigned int OutWidth, OutHeight;
	NewFont->Texture = PlatformFileSystem::LoadBitmapFromFile(NewFont->TextureAtlasFileName, OutWidth, OutHeight);

	fclose(FilePointer);

	return NewFont;
}

void UnloadFontLibrary()
{
	for (unsigned int i = 0; i < FONT_LIBRARY_SIZE; ++i)
	{
		if (LoadedFonts[i])
		{
			delete LoadedFonts[i]->FontImage;
			delete LoadedFonts[i];
		}
	}
	delete[] LoadedFonts;
}
