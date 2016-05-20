#include "World.h"

#include "glm\common.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "../Engine/Block.h"
#include "../Engine/Chunk.h"
#include "../Engine/PerlinNoise.h"

#include "../Engine/ScriptEngine.h"
#include "../Engine/ChunkManager.h"
#include "../Platform/Platform.h"

#include "../Engine/Core/Memory.h"

#define CHUNK_LOADING_RADIUS 2

#define TOCHUNK_COORD(X, Y, Z) X / (int) Octree<Voxel>::SIZE, Y / (int) Octree<Voxel>::SIZE, Z / (int) Octree<Voxel>::SIZE

extern GameMemoryManager* g_MemoryManager;

World::World()
{
	m_pCurrentPlayer = NULL;
	CachedChunk = NULL;
}

World::~World()
{
	if (m_pCurrentPlayer)
	{
		delete m_pCurrentPlayer;
	}
}

void World::Initialize()
{
	BlockManager::SetupBlocks();

	m_pCurrentPlayer = new Player();
	m_pCurrentPlayer->BeginPlay();
	m_pCurrentPlayer->m_pWorldObject = this;

	m_pChunkManager = new ChunkManager(this, 2);
	m_pChunkManager->LoadNewChunks(glm::ivec3(0, 0, 0));

	Script WorldGen = Script("world_gen.lua");
	WorldGen.CallFunction("gen_world");
}

void World::Update(float DeltaTime)
{
	glm::ivec3 PlayerChunkPosition = m_pCurrentPlayer->m_pPlayerCamera->Position / glm::vec3(Octree<Voxel>::SIZE);

	// Unload/load chunks
	m_pChunkManager->LoadNewChunks(PlayerChunkPosition);
	m_pChunkManager->UnloadChunks(PlayerChunkPosition);

	// Tick the chunks
	m_pChunkManager->Tick(DeltaTime);

	// Update the player last as chunk updates should take precedence
	m_pCurrentPlayer->Update(DeltaTime);
}

Chunk* World::GetLoadedChunk(const int& ChunkX, const int& ChunkY, const int& ChunkZ)
{
	if (CachedChunk && 
		CachedChunk->m_ChunkX == ChunkX &&
		CachedChunk->m_ChunkY == ChunkY &&
		CachedChunk->m_ChunkZ == ChunkZ)
	{
		return CachedChunk;
	}
	else
	{
		CachedChunk = m_pChunkManager->GetChunkAt(ChunkX, ChunkY, ChunkZ);
		return CachedChunk;
	}
}

Voxel* World::GetBlock(const int& X, const int& Y, const int& Z)
{
	Chunk* QueriedChunk = GetLoadedChunk(X / (int) Octree<Voxel>::SIZE, Y / (int) Octree<Voxel>::SIZE, Z / (int) Octree<Voxel>::SIZE);
	return QueriedChunk != NULL ? QueriedChunk->GetVoxel(X % Octree<Voxel>::SIZE, Y % Octree<Voxel>::SIZE, Z % Octree<Voxel>::SIZE) : NULL;;
}

void World::AddBlock(const int& X, const int& Y, const int& Z, const unsigned int& BlockID)
{
	assert(BlockID != 0);

	Chunk* ChunkToAddTo = GetLoadedChunk(TOCHUNK_COORD(X, Y, Z));
	if (ChunkToAddTo)
	{
		// This gets the local coordinate in the chunks local coordinate
		// system, which ranges from 0 to 31
		int LocalX = abs(X) % Octree<Voxel>::SIZE;
		int LocalY = abs(Y) % Octree<Voxel>::SIZE;
		int LocalZ = abs(Z) % Octree<Voxel>::SIZE;

		ChunkToAddTo->SetVoxel(this, LocalX, LocalY, LocalZ, BlockID);
	}
}

void World::RemoveBlock(const int & X, const int & Y, const int & Z)
{
	Chunk* QueriedChunk = GetLoadedChunk(TOCHUNK_COORD(X, Y, Z));
	if (QueriedChunk)
	{
		// This gets the local coordinate in the chunks local coordinate
		// system, which ranges from 0 to 31
		int LocalX = X % Octree<Voxel>::SIZE;
		int LocalY = Y % Octree<Voxel>::SIZE;
		int LocalZ = Z % Octree<Voxel>::SIZE;

		QueriedChunk->RemoveVoxel(this, LocalX, LocalY, LocalZ);
	}
}

Voxel* World::GetMultiblock(const int &X, const int &Y, const int &Z)
{
	int ChunkX = X / (int) Octree<Voxel>::SIZE;
	int ChunkY = Y / (int) Octree<Voxel>::SIZE;
	int ChunkZ = Z / (int) Octree<Voxel>::SIZE;

	Chunk* QueriedChunk = GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
	if (QueriedChunk)
	{
		// This gets the local coordinate in the chunks local coordinate
		// system, which ranges from 0 to 31
		int LocalX = X % Octree<Voxel>::SIZE;
		int LocalY = Y % Octree<Voxel>::SIZE;
		int LocalZ = Z % Octree<Voxel>::SIZE;

		Voxel* QueriedVoxel = QueriedChunk->GetVoxel(LocalX, LocalY, LocalZ);
		// If we got the child node, set the block operated upon to be the parent (master)
		if (QueriedVoxel->Parent != NULL)
		{
			QueriedVoxel = QueriedVoxel->Parent;
		}
		// TODO: Shuold we check whether this actually is a multiblock or not
		// and if it's not return NULL?
		return QueriedVoxel;
	}
	return NULL;
}

void World::AddMultiblock(const int &X, const int &Y, const int &Z, const unsigned int &BlockID)
{
	BlockInfo BlockIDInfo = BlockManager::LoadedBlocks[BlockID];
	unsigned int MultiblockWidth = BlockIDInfo.RenderData.MultiblockRenderData.Width;
	unsigned int MultiblockHeight = BlockIDInfo.RenderData.MultiblockRenderData.Height;
	unsigned int MultiblockDepth = BlockIDInfo.RenderData.MultiblockRenderData.Depth;

	for (unsigned int XCoord = 0; XCoord < MultiblockWidth; ++XCoord)
	{
		for (unsigned int YCoord = 0; YCoord < MultiblockHeight; ++YCoord)
		{
			for (unsigned int ZCoord = 0; ZCoord < MultiblockDepth; ++ZCoord)
			{
				Voxel *CurrentBlock = GetBlock(X + XCoord, Y + YCoord, Z + ZCoord);
				if (CurrentBlock)
				{
					// We can't place a multiblock here since the space is
					// already occupied
					return;
				}
			}
		}
	}

	int ChunkX = X / (int) Octree<Voxel>::SIZE;
	int ChunkY = Y / (int) Octree<Voxel>::SIZE;
	int ChunkZ = Z / (int) Octree<Voxel>::SIZE;

	Chunk *QueriedChunk = GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
	if (QueriedChunk)
	{
		// This gets the local coordinate in the chunks local coordinate
		// system, which ranges from 0 to 31
		int LocalX = X % Octree<Voxel>::SIZE;
		int LocalY = Y % Octree<Voxel>::SIZE;
		int LocalZ = Z % Octree<Voxel>::SIZE;

		Voxel *Parent = NULL;
		for (unsigned int XCoord = 0; XCoord < MultiblockWidth; ++XCoord)
		{
			for (unsigned int YCoord = 0; YCoord < MultiblockHeight; ++YCoord)
			{
				for (unsigned int ZCoord = 0; ZCoord < MultiblockDepth; ++ZCoord)
				{
					if (LocalX >= 16)
					{
						++ChunkX;
						QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
						LocalX %= 16;
					}
					if (LocalY >= 16)
					{
						++ChunkY;
						QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
						LocalY %= 16;
					}
					if (LocalZ >= 16)
					{
						++ChunkZ;
						QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
						LocalZ %= 16;
					}

					if (XCoord == 0 && YCoord == 0 && ZCoord == 0)
					{
						Parent = QueriedChunk->GetVoxel(LocalX, LocalY, LocalZ);
					}
					QueriedChunk->SetVoxel(this, LocalX, LocalY, LocalZ, BlockID, Parent);

					++LocalZ;
				}
				++LocalY;
			}
			++LocalZ;
		}
	}
}

void World::RemoveMultiblock(const int& X, const int& Y, const int& Z)
{
	Voxel *Parent = GetMultiblock(X, Y, Z);

	if (Parent)
	{
		int ChunkX = X / (int) Octree<Voxel>::SIZE;
		int ChunkY = Y / (int) Octree<Voxel>::SIZE;
		int ChunkZ = Z / (int) Octree<Voxel>::SIZE;

		Chunk *QueriedChunk = GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
		if (QueriedChunk)
		{
			int LocalX = Parent->LocalPosX;
			int LocalY = Parent->LocalPosY;
			int LocalZ = Parent->LocalPosZ;

			BlockInfo BlockIDInfo = BlockManager::LoadedBlocks[Parent->BlockID];
			unsigned int MultiblockWidth = BlockIDInfo.RenderData.MultiblockRenderData.Width;
			unsigned int MultiblockHeight = BlockIDInfo.RenderData.MultiblockRenderData.Height;
			unsigned int MultiblockDepth = BlockIDInfo.RenderData.MultiblockRenderData.Depth;

			for (unsigned int XCoord = 0; XCoord < MultiblockWidth; ++XCoord)
			{
				for (unsigned int YCoord = 0; YCoord < MultiblockHeight; ++YCoord)
				{
					for (unsigned int ZCoord = 0; ZCoord < MultiblockDepth; ++ZCoord)
					{
						++LocalZ;

						if (LocalX >= 16)
						{
							++ChunkX;
							QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
							LocalX %= 16;
						}
						if (LocalY >= 16)
						{
							++ChunkY;
							QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
							LocalY %= 16;
						}
						if (LocalZ >= 16)
						{
							++ChunkZ;
							QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
							LocalZ %= 16;
						}

						QueriedChunk->RemoveVoxel(this, LocalX, LocalY, LocalZ);
					}
					++LocalY;
				}
				++LocalZ;
			}
		}
	}
}
