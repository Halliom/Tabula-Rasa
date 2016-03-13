#pragma once

#include "../Rendering/GL_shader.h"
#include "../Rendering/TextRenderer.h"

#include "Player.h"

#include "../Engine/Chunk.h"
#include "../Engine/Octree.h"

class World
{
public:
	World();

	~World();

	void Initialize();

	void Update(float DeltaTime);

	Voxel* GetBlock(const int& X, const int& Y, const int& Z);

	void AddBlock(const int& X, const int& Y, const int& Z, const unsigned int& BlockID);

	void RemoveBlock(const int& X, const int& Y, const int& Z);

	Chunk* LoadChunk(const int& ChunkX, const int& ChunkY, const int& ChunkZ);

	Player* CurrentPlayer;

private:

	Octree<Chunk> m_LoadedChunks;

	int ChunkLoadingCenterX;
	int ChunkLoadingCenterY;
	int ChunkLoadingCenterZ;

	GLShaderProgram* Shader;
};