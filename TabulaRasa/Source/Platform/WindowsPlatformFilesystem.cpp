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

GLuint PlatformFileSystem::LoadBitmapFromFile(char* FileName)
{
	GLuint Result;
	FILE* ImageFile = fopen(FileName, "r");

	glGenTextures(1, &Result);
	fread();
}

void PlatformFileSystem::LoadModel(LoadedModel *Model, const char *FileName)
{
	glGenVertexArrays(1, &Model->m_AssetVAO);
	glBindVertexArray(Model->m_AssetVAO);

	glGenBuffers(1, &Model->m_AssetVBO);
	glGenBuffers(1, &Model->m_AssetIBO);

	// Get the actual location on disk
	std::string FileLocation = GetAssetDirectory(DT_MODELS);
	FileLocation.append(FileName);

	Assimp::Importer Importer;
	const aiScene *ImportedScene = Importer.ReadFile(
		FileLocation.c_str(),
		aiProcess_Triangulate |
		aiProcess_FlipWindingOrder |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords);

	if (!ImportedScene)
	{
		const char *ErrorMessage = Importer.GetErrorString();
		// TODO: Error loggin
		return;
	}

	std::vector<float> Vertices;
	std::vector<unsigned short> Indices;
	unsigned short Offset = 0;
	for (unsigned int i = 0; i < ImportedScene->mNumMeshes; ++i)
	{
		Vertices.reserve(Vertices.size() + 6 * ImportedScene->mMeshes[i]->mNumVertices);
		for (unsigned int VertexID = 0; VertexID < ImportedScene->mMeshes[i]->mNumVertices; ++VertexID)
		{
			Vertices.push_back(ImportedScene->mMeshes[i]->mVertices[VertexID].x);
			Vertices.push_back(ImportedScene->mMeshes[i]->mVertices[VertexID].z);
			Vertices.push_back(ImportedScene->mMeshes[i]->mVertices[VertexID].y);
			Vertices.push_back(ImportedScene->mMeshes[i]->mNormals[VertexID].x);
			Vertices.push_back(ImportedScene->mMeshes[i]->mNormals[VertexID].z);
			Vertices.push_back(ImportedScene->mMeshes[i]->mNormals[VertexID].y);
		}
		Indices.reserve(Indices.size() + 3 * ImportedScene->mMeshes[i]->mNumFaces);
		for (unsigned int FaceID = 0; FaceID < ImportedScene->mMeshes[i]->mNumFaces; ++FaceID)
		{
			// Since all faces are triangles due to aiProcess_Triangulate there will only be
			// 3 indices per face
			Indices.push_back(Offset + (unsigned short) ImportedScene->mMeshes[i]->mFaces[FaceID].mIndices[2]);
			Indices.push_back(Offset + (unsigned short) ImportedScene->mMeshes[i]->mFaces[FaceID].mIndices[1]);
			Indices.push_back(Offset + (unsigned short) ImportedScene->mMeshes[i]->mFaces[FaceID].mIndices[0]);
		}
		Offset = Vertices.size() / 6;
	}

	glBindBuffer(GL_ARRAY_BUFFER, Model->m_AssetVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Vertices.size(), Vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Model->m_AssetIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * Indices.size(), Indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // Position
	glEnableVertexAttribArray(1); // Normal

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void *) (sizeof(float) * 3));

	Model->m_NumVertices = Indices.size();

	glBindVertexArray(0);
}
