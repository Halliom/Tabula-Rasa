#include "WindowsPlatformFilesystem.h"

#include "windows.h"
#include <fstream>

#include "picopng.cpp"
#include <assimp/Importer.hpp>
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#define MAX_FILE_READ_SIZE 8192

std::string PlatformFileSystem::LoadFile(const AssetDirectoryType& Directory, const char* FileName)
{
	std::string AssetDir = GetAssetDirectory(Directory);
	AssetDir.append(FileName);

	HANDLE FileHandle = CreateFileA(
		AssetDir.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		DWORD ErrorCode = GetLastError();

		//return "";
	}

	char FileContents[MAX_FILE_READ_SIZE];
	unsigned long NumberOfBytesRead;

	bool Success = ReadFile(
		FileHandle,
		FileContents,
		MAX_FILE_READ_SIZE,
		&NumberOfBytesRead,
		NULL);

	if (!Success)
	{
		DWORD ErrorCode = GetLastError();
	}

	CloseHandle(FileHandle);

	return std::string(FileContents, NumberOfBytesRead);
}

std::string PlatformFileSystem::GetAssetDirectory(const AssetDirectoryType& Directory)
{
	char CurrentDirectory[MAX_PATH];
	unsigned int NumberOfBytesRead = GetCurrentDirectoryA(MAX_PATH, CurrentDirectory);

	std::string OutDirectory = std::string(CurrentDirectory, NumberOfBytesRead);

	//TODO: Cache this
	switch (Directory)
	{
		case DT_SHADERS:
		{
			OutDirectory.append("\\Assets\\Shaders\\");
			break;
		}
		case DT_FONTS:
		{
			OutDirectory.append("\\Assets\\Fonts");
			break;
		}
		case DT_TEXTURES:
		{
			OutDirectory.append("\\Assets\\Textures\\");
			break;
		}
		case DT_MODELS:
		{
			OutDirectory.append("\\Assets\\Models\\");
			break;
		}
		default: { break; }
	}
	return OutDirectory;
}

std::vector<unsigned char>* PlatformFileSystem::LoadImageFromFile(const std::string& FileName, unsigned int& OutWidth, unsigned int& OutHeight)
{
	std::ifstream File;
	File.open(FileName, std::ios::binary);

	File.seekg(0, std::ios::end);
	size_t Size = File.tellg();
	File.seekg(0, std::ios::beg);

	std::vector<unsigned char> Data;
	//Reduce the file size by any header bytes that might be present
	Size -= File.tellg();
	Data.resize(Size);
	File.read((char*) &(Data[0]), Size);
	File.close();

	std::vector<unsigned char>* Pixels = new std::vector<unsigned char>();
	unsigned long Width;
	unsigned long Height;

	decodePNG(*Pixels, Width, Height, &(Data[0]), Size);

	OutWidth = Width;
	OutHeight = Height;
	return Pixels;
}

LoadedModel* PlatformFileSystem::LoadModel(const char *FileName)
{
	LoadedModel* Result = new LoadedModel();

	glGenVertexArrays(1, &Result->m_AssetVAO);
	glBindVertexArray(Result->m_AssetVAO);

	glGenBuffers(1, &Result->m_AssetVBO);
	glGenBuffers(1, &Result->m_AssetIBO);

	// Get the actual location on disk
	std::string FileLocation = GetAssetDirectory(DT_MODELS);
	FileLocation.append(FileName);

	Assimp::Importer Importer;
	const aiScene *ImportedScene = Importer.ReadFile(
		FileLocation.c_str(),
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords);

	if (!ImportedScene)
	{
		const char *ErrorMessage = Importer.GetErrorString();
		// TODO: Error loggin
		return NULL;
	}

	std::vector<float> Vertices;
	for (unsigned int i = 0; i < ImportedScene->mNumMeshes; ++i)
	{
		for (int VertexID = 0; VertexID < ImportedScene->mMeshes[i]->mNumVertices; ++VertexID)
		{
			Vertices.push_back(ImportedScene->mMeshes[i]->mVertices[VertexID].x);
			Vertices.push_back(ImportedScene->mMeshes[i]->mVertices[VertexID].y);
			Vertices.push_back(ImportedScene->mMeshes[i]->mVertices[VertexID].z);
			Vertices.push_back(ImportedScene->mMeshes[i]->mNormals[VertexID].x);
			Vertices.push_back(ImportedScene->mMeshes[i]->mNormals[VertexID].y);
			Vertices.push_back(ImportedScene->mMeshes[i]->mNormals[VertexID].z);
		}
	}

	ImportedScene->mMeshes[0];

	return Result;
}
