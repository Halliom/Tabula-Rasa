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

AChunk* AChunkManager::GetOrCreateChunkFromWorldPosition(FWorldPosition Position)
{
	if (Position.PositionX != 0 && Position.PositionY != 0 && Position.PositionZ != 0)
	{
		int32 ChunkX = FMath::CeilToInt((float) Position.PositionX / (float) INITIAL_CHUNK_SIZE) * (Position.PositionX > 0 ? 1 : -1);
		int32 ChunkY = FMath::CeilToInt((float) Position.PositionY / (float) INITIAL_CHUNK_SIZE) * (Position.PositionY > 0 ? 1 : -1);
		int32 ChunkZ = FMath::CeilToInt((float) Position.PositionZ / (float) INITIAL_CHUNK_SIZE) * (Position.PositionZ > 0 ? 1 : -1);
		
		ChunkPos ChunkPosition = ChunkPos(ChunkX, ChunkY, ChunkZ);
		AChunk* Chunk = LoadedChunks.FindRef(ChunkPosition); //TODO: Fix crash here
		if (Chunk)
		{
			return Chunk;
		}
		else
		{
			Chunk = Cast<AChunk>(UGameplayStatics::BeginSpawningActorFromClass(this,
				AChunk::StaticClass(),
				FTransform(FVector(ChunkX * INITIAL_CHUNK_SIZE, ChunkY * INITIAL_CHUNK_SIZE, ChunkZ * INITIAL_CHUNK_SIZE))));
			if (Chunk)
			{
				Chunk->ChunkPosition = FWorldPosition(ChunkX * INITIAL_CHUNK_SIZE, ChunkY * INITIAL_CHUNK_SIZE, ChunkZ * INITIAL_CHUNK_SIZE);

				// Add it (save it) to the LoadedChunks map
				LoadedChunks.Add(ChunkPosition, Chunk);

				return Chunk;
			}
		}
	}
	return NULL;
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