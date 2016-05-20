#include "ChunkManager.h"

#include "Core\Memory.h"
#include "../Rendering/ChunkRenderer.h"

ChunkManager::ChunkManager(World* WorldObject, int ChunkLoadingRadius) : 
	m_ChunkLoadingRadius(ChunkLoadingRadius),
	m_pWorldObject(WorldObject)
{
	m_LoadedChunks.reserve(ChunkLoadingRadius * ChunkLoadingRadius * ChunkLoadingRadius);
}

ChunkManager::~ChunkManager()
{
	for (auto It = m_LoadedChunks.begin(); It != m_LoadedChunks.end(); ++It)
	{
		Chunk* Chunk = (*It).second;
		if (Chunk != NULL)
		{
			// TODO: Free the Chunk->m_pChunkRenderData back aswell
			g_MemoryManager->m_pChunkAllocator->Deallocate(Chunk);
		}
	}
}

void ChunkManager::Tick(float DeltaTime)
{
	for (auto It = m_LoadedChunks.begin(); It != m_LoadedChunks.end(); ++It)
	{
		if ((*It).second == NULL)
			continue;

		(*It).second->Tick(DeltaTime);
	}
}

Chunk* ChunkManager::LoadChunk(glm::ivec3 ChunkPosition)
{
	Chunk* Result = g_MemoryManager->m_pChunkAllocator->AllocateNew();
	Result->m_ChunkX = ChunkPosition.x;
	Result->m_ChunkY = ChunkPosition.y;
	Result->m_ChunkZ = ChunkPosition.z;
	Result->Initialize();

	return Result;
}

void ChunkManager::UnloadChunks(glm::ivec3 PlayerChunkPosition)
{
	auto It = m_LoadedChunks.begin();
	while (It != m_LoadedChunks.end())
	{
		if ((*It).second == NULL)
			continue;

		Chunk* CurrentChunk = (*It).second;
		if (CurrentChunk != NULL)
		{
			glm::ivec3 ChunkPosition = (*It).first;
			float DistanceFromPlayer = glm::length(glm::vec3(ChunkPosition - PlayerChunkPosition));
			if (DistanceFromPlayer > m_ChunkLoadingRadius)
			{
				// The chunk is outside the radius and needs to be removed
				It = m_LoadedChunks.erase(It);

				// TODO: Free the Chunk->m_pChunkRenderData back aswell
				g_MemoryManager->m_pChunkAllocator->Deallocate(CurrentChunk);
				continue;
			}
		}
		It++;
	}
}

void ChunkManager::LoadNewChunks(glm::ivec3 PlayerChunkPosition)
{
	for (int i = -m_ChunkLoadingRadius; i < m_ChunkLoadingRadius; ++i)
	{
		for (int j = -m_ChunkLoadingRadius; j < m_ChunkLoadingRadius; ++j)
		{
			for (int k = -m_ChunkLoadingRadius; k < m_ChunkLoadingRadius; ++k)
			{
				glm::ivec3 LocalChunkPosition = glm::ivec3(i, j, k);
				float DistanceFromPlayer = glm::length(glm::vec3(LocalChunkPosition));
				if (DistanceFromPlayer <= m_ChunkLoadingRadius)
				{
					if (m_LoadedChunks.find(LocalChunkPosition + PlayerChunkPosition) == m_LoadedChunks.end())
					{
						// There is no chunk in this position, load ti
						glm::ivec3 ChunkPosition = LocalChunkPosition + PlayerChunkPosition;
						m_LoadedChunks.insert({ ChunkPosition, LoadChunk(ChunkPosition) });
					}
				}
			}
		}
	}
}

Chunk* ChunkManager::GetChunkAt(glm::ivec3 ChunkPosition)
{
	auto Chunk = m_LoadedChunks.find(ChunkPosition);
	if (Chunk != m_LoadedChunks.end())
	{
		return (*Chunk).second;
	}
	else
	{
		return NULL;
	}
}

Chunk* ChunkManager::GetChunkAt(int ChunkPositionX, int ChunkPositionY, int ChunkPositionZ)
{
	return GetChunkAt(glm::ivec3(ChunkPositionX, ChunkPositionY, ChunkPositionZ));
}

List<Chunk*> ChunkManager::GetVisibleChunks(glm::ivec3 PlayerChunkPosition)
{
	List<Chunk*> Result;

	for (auto It = m_LoadedChunks.begin(); It != m_LoadedChunks.end(); ++It)
	{
		Chunk* Chunk = (*It).second;
		if (Chunk != NULL)
		{
			Result.Push(Chunk);
		}
	}
	return Result;
}
