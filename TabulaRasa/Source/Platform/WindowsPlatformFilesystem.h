#pragma once

#include <string>
#include <vector>

#include "GL\glew.h"

enum AssetDirectoryType
{
	DT_SHADERS,
	DT_FONTS,
	DT_TEXTURES,
	DT_MODELS
};

struct LoadedModel
{
	/**
	 * Vertex data
	 */
	GLuint m_AssetVAO;
	GLuint m_AssetVBO;
	GLuint m_AssetIBO;

	/**
	 * Data specific to rendering of the model
	 */
	unsigned int m_NumVertices;
};

class PlatformFileSystem
{
public:
	static std::string LoadFile(const AssetDirectoryType& Directory, const char* FileName);

	static __forceinline std::string LoadFile(const AssetDirectoryType& Directory, const std::string& FileName)
	{
		return LoadFile(Directory, FileName.c_str());
	}

	static std::string GetAssetDirectory(const AssetDirectoryType& Directory);

	static std::vector<unsigned char> *LoadImageFromFile(const std::string& FileName, unsigned int& OutWidth, unsigned int& OutHeight);

	static void LoadModel(LoadedModel *Model, const char* FileName);
};
