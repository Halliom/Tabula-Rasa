#pragma once

#include "../Rendering/GuiSystem.h"

enum VoxelSide : uint32_t;

struct RayHitResult
{
	/* 
	 * This holds the position of the voxel that was hit
	 */
	glm::ivec3		Position;

	/*
	 * The ID of the block that was hit or 0 if nothing was hit
	 */
	unsigned int	BlockID;

	/*
	 * This shows which side (of the block that was hit) was hit
	 */
    VoxelSide       Side;
};

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

	RayHitResult RayTraceVoxels(const struct Ray& Ray);

	class Player*			m_pCurrentPlayer;
	class ChunkManager*		m_pChunkManager;
	class WorldGenerator*	m_pWorldGenerator;

private:

	class Chunk*		CachedChunk;
};