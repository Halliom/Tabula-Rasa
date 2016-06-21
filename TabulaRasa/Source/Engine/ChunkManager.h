#pragma once
#include <unordered_map>

#include "glm\common.hpp"

#include "Octree.h"
#include "Chunk.h"

#define IVEC_HASH_PRIME 5503

namespace std
{
	template<>
	struct hash<glm::ivec3>
	{
		size_t operator()(const glm::ivec3& val) const noexcept
		{
			return (
				(IVEC_HASH_PRIME + std::hash<int>()(val.x)) *
				IVEC_HASH_PRIME + std::hash<int>()(val.y)) *
				IVEC_HASH_PRIME + std::hash<int>()(val.z)
				;
		}
	};

	template<>
	struct equal_to<glm::ivec3>
	{
		bool operator()(const glm::ivec3& val0, const glm::ivec3& val1) const
		{
			return val0.x == val1.x && 
				   val0.y == val1.y && 
				   val0.z == val1.z;
		}
	};
}
// ((51 + int_hash(row)) * 51 + int_hash(col)) * 51 + int_hash(z)

class ChunkManager
{
public:

	ChunkManager(World* WorldObject, class WorldGenerator* WorldGen, int ChunkLoadingRadius);

	~ChunkManager();

	void Tick(float DeltaTime);

	Chunk* LoadChunk(glm::ivec3 ChunkPosition);

	void UnloadChunks(glm::ivec3 PlayerChunkPosition);

	void LoadNewChunks(glm::ivec3 PlayerChunkPosition);

	Chunk* GetChunkAt(glm::ivec3 ChunkPosition);

	Chunk* GetChunkAt(int ChunkPositionX, int ChunkPositionY, int ChunkPositionZ);

	List<Chunk*> GetVisibleChunks(glm::ivec3 PlayerChunkPosition);

	int m_ChunkLoadingRadius;

private:

	std::unordered_map<glm::ivec3, Chunk*>	m_LoadedChunks;

	World*									m_pWorldObject;
	WorldGenerator*							m_pWorldGenerator;
};