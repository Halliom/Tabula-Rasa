#include "VoxelLibPluginPrivatePCH.h"

#include "ChunkManager.h"

AChunkManager* AChunkManager::ChunkManager = NULL;

AChunkManager* AChunkManager::GetStaticChunkManager()
{
	return AChunkManager::ChunkManager;
}

AChunkManager::AChunkManager(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	if (AChunkManager::ChunkManager == NULL)
	{
		AChunkManager::ChunkManager = this;
	}
}

AChunkManager::~AChunkManager()
{
	for (auto It = LoadedChunks.CreateConstIterator(); It; ++It)
	{
		AChunk* RemovedChunk = NULL;
		LoadedChunks.RemoveAndCopyValue((*It).Key, RemovedChunk);
		if (RemovedChunk)
		{
			delete RemovedChunk;
		}
	}
	LoadedChunks.Empty();
	if (AChunkManager::ChunkManager == this)
	{
		AChunkManager::ChunkManager = NULL;
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

AChunk* AChunkManager::GetOrCreateChunkFromWorldPosition(AActor* ParentActor, FWorldPosition Position)
{
	int32 ChunkX = FMath::CeilToInt((float)Position.PositionX / (float)INITIAL_CHUNK_SIZE) * (Position.PositionX > 0 ? 1 : -1);
	int32 ChunkY = FMath::CeilToInt((float)Position.PositionY / (float)INITIAL_CHUNK_SIZE) * (Position.PositionY > 0 ? 1 : -1);
	int32 ChunkZ = FMath::CeilToInt((float)Position.PositionZ / (float)INITIAL_CHUNK_SIZE) * (Position.PositionZ > 0 ? 1 : -1);

	if (Position.PositionX == 0)
		ChunkX = 1;
	if (Position.PositionY == 0)
		ChunkY = 1;
	if (Position.PositionZ == 0)
		ChunkZ = 1;

	ChunkPos ChunkPosition = ChunkPos(ChunkX, ChunkY, ChunkZ);
	AChunk* Chunk = LoadedChunks.FindRef(ChunkPosition);
	if (Chunk)
	{
		return Chunk;
	}
	else
	{
		Chunk = ParentActor->GetWorld()->SpawnActor<AChunk>();
		if (Chunk)
		{
			Chunk->ChunkPosition = FWorldPosition(ChunkX, ChunkY, ChunkZ);

			// Add it (save it) to the LoadedChunks map
			LoadedChunks.Add(ChunkPosition, Chunk);
			return Chunk;
		}
		return NULL;
	}
}

void AChunkManager::AddChunk(FWorldPosition ChunkPosition, AChunk* Chunk)
{
	ChunkPos Pos(ChunkPosition);
	check(!LoadedChunks.Contains(Pos));
	check(ChunkPosition.PositionX != 0 && ChunkPosition.PositionY != 0 && ChunkPosition.PositionZ != 0);

	LoadedChunks.Add(Pos, Chunk);
}

bool AChunkManager::DeleteChunkAtPosition(FWorldPosition ChunkPosition)
{
	ChunkPos Pos(ChunkPosition);

	if (LoadedChunks.Contains(Pos))
	{
		LoadedChunks.Remove(Pos);
		return true;
	}
	return false;
}
