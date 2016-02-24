#pragma once

#include <string>
#include <vector>

enum AssetDirectoryType
{
	DT_SHADERS,
	DT_FONTS
};

class PlatformFileSystem
{
public:
	static std::string LoadFile(const AssetDirectoryType& Directory, const char* FileName);

	static std::string LoadFile(const AssetDirectoryType& Directory, const std::string& FileName)
	{
		return LoadFile(Directory, FileName.c_str());
	}

	static std::string* GetAssetDirectory(const AssetDirectoryType& Directory);

	static std::vector<unsigned char>* LoadImageFromFile(const std::string& FileName, unsigned int& OutWidth, unsigned int& OutHeight);
};