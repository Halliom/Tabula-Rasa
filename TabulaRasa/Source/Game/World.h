#pragma once

#include "../Rendering/GL_shader.h"
#include "../Rendering/TextRenderer.h"
#include "Player.h"

class World
{
public:
	World();

	~World();

	void Initialize();

	void Update(float DeltaTime);

	class Chunk* GetLoadedChunk(const int &ChunkX, const int &ChunkY, const int &ChunkZ);

	class Voxel* GetBlock(const int& X, const int& Y, const int& Z);

	void AddBlock(const int& X, const int& Y, const int& Z, const unsigned int& BlockID);

	void RemoveBlock(const int& X, const int& Y, const int& Z);

	class Voxel* GetMultiblock(const int& X, const int& Y, const int& Z);

	void AddMultiblock(const int& X, const int& Y, const int& Z, const unsigned int& BlockID);

	void RemoveMultiblock(const int& X, const int& Y, const int& Z);

	Player* m_pCurrentPlayer;

	class ChunkManager* m_pChunkManager;

private:

	class Chunk* CachedChunk;
};