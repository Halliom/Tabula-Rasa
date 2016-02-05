#include "VoxelLibPluginPrivatePCH.h"

#include "ChunkManager.h"

AChunkManager* AChunkManager::ChunkManager = NULL;

AChunkManager* AChunkManager::GetStaticChunkManager()
{
	return AChunkManager::ChunkManager;
}

AChunkManager::AChunkManager(const FObjectInitializer& ObjectIntiailizer)
{
	if (AChunkManager::ChunkManager == NULL)
	{
		AChunkManager::ChunkManager = this;
	}
}

AChunk* AChunkManager::GetChunkFromPosition(FWorldPosition Position)
{
	int32 ChunkX = Position.PositionX / INITIAL_CHUNK_SIZE;
	int32 ChunkY = Position.PositionY / INITIAL_CHUNK_SIZE;
	int32 ChunkZ = Position.PositionZ / INITIAL_CHUNK_SIZE;

	ChunkPos ChunkPos = { ChunkX, ChunkY, ChunkZ };

	return *(LoadedChunks.Find(ChunkPos));
}

void AChunkManager::AddChunk(FWorldPosition ChunkPosition, AChunk* Chunk)
{
	ChunkPos Pos(ChunkPosition);
	check(!LoadedChunks.Contains(Pos));
	check(ChunkPosition.PositionX != 0 && ChunkPosition.PositionY != 0 && ChunkPosition.PositionZ != 0);

	LoadedChunks.Add(Pos, Chunk);
}