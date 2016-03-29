#include "World.h"

#include "glm\common.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "../Engine/Block.h"
#include "../Engine/PerlinNoise.h"

#include "../Rendering/ChunkRenderer.h"

#define CHUNK_LOADING_RADIUS 4

#define TOCHUNK_COORD(X, Y, Z) X / (int) Octree<Voxel>::SIZE, Y / (int) Octree<Voxel>::SIZE, Z / (int) Octree<Voxel>::SIZE

World::World()
{
	CurrentPlayer = NULL;
	CachedChunk = NULL;
}

World::~World()
{
	if (CurrentPlayer)
	{
		delete CurrentPlayer;
	}
}

void World::Initialize()
{
	BlockManager::SetupBlocks();

	CurrentPlayer = new Player();
	CurrentPlayer->BeginPlay();

	ChunkLoadingCenterX = 0;
	ChunkLoadingCenterY = 0;
	ChunkLoadingCenterZ = 0;

	for (int i = 0; i < CHUNK_LOADING_RADIUS; ++i)
	{
		for (int j = 0; j < CHUNK_LOADING_RADIUS; ++j)
		{
			for (int k = 0; k < CHUNK_LOADING_RADIUS; ++k)
			{
				//TODO: This might be thrashing the cache a bit
				int Index = i + CHUNK_LOADING_RADIUS * (j + CHUNK_LOADING_RADIUS * k);
				int HalfChunkRadius = CHUNK_LOADING_RADIUS / 2;
				Chunk* ChunkToInsert = LoadChunk(
					ChunkLoadingCenterX + i - HalfChunkRadius,
					ChunkLoadingCenterY + j - HalfChunkRadius,
					ChunkLoadingCenterZ + k - HalfChunkRadius);
				m_LoadedChunks.InsertNode(glm::vec3(i, j, k), ChunkToInsert);
			}
		}
	}

	//PerlinNoise NoiseFunction;

#if 1
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			for (int k = 0; k < 16; ++k)
			{
				if (rand() % 10 > 5)
					AddBlock(i, j, k, BLOCK_ID_GRASS);
			}
		}
	}
#else
	AddBlock(0, 0, 0, BLOCK_ID_GRASS);
	AddBlock(0, 0, 1, BLOCK_ID_GRASS);
	AddBlock(0, 0, 2, BLOCK_ID_GRASS);
	AddBlock(1, 0, 0, BLOCK_ID_GRASS);
	AddBlock(1, 0, 1, BLOCK_ID_GRASS);
	AddBlock(1, 0, 2, BLOCK_ID_GRASS);
	AddBlock(2, 0, 0, BLOCK_ID_GRASS);
	AddBlock(2, 0, 1, BLOCK_ID_GRASS);
	AddBlock(2, 0, 2, BLOCK_ID_GRASS);

	AddBlock(4, 0, 4, BLOCK_ID_GRASS);
	AddBlock(4, 1, 4, BLOCK_ID_GRASS);
	AddBlock(4, 0, 5, BLOCK_ID_GRASS);
	AddBlock(4, 1, 5, BLOCK_ID_GRASS);

	AddBlock(1, 1, 1, BLOCK_ID_GRASS);

	AddMultiblock(2, 2, 2, BLOCK_ID_CHEST);
#endif
}

void World::Update(float DeltaTime)
{
	CurrentPlayer->Update(DeltaTime);

	// TODO: Update all the chunks
	for (auto& It : m_LoadedChunks.Nodes)
	{
		if (It.second->m_pNodeData == NULL)
			continue;

		// If the NodeData's render state is dirty
		if (It.second->m_pNodeData->m_bIsRenderStateDirty)
		{
			ChunkRenderData* RenderData = It.second->m_pNodeData->m_pChunkRenderData;
			ChunkRenderer::UpdateRenderData(RenderData, It.second->m_pNodeData);
			It.second->m_pNodeData->m_bIsRenderStateDirty = false;
		}
	}
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
		int HalfChunkRadius = CHUNK_LOADING_RADIUS / 2;
		if ((ChunkX >= ChunkLoadingCenterX - HalfChunkRadius && ChunkX < ChunkLoadingCenterX + HalfChunkRadius) &&
			(ChunkY >= ChunkLoadingCenterY - HalfChunkRadius && ChunkY < ChunkLoadingCenterY + HalfChunkRadius) &&
			(ChunkZ >= ChunkLoadingCenterZ - HalfChunkRadius && ChunkZ < ChunkLoadingCenterZ + HalfChunkRadius))
		{
			// Do the inverse of what we do when adding/creating chunks
			int RelativeX = ChunkX - ChunkLoadingCenterX + HalfChunkRadius;
			int RelativeY = ChunkY - ChunkLoadingCenterY + HalfChunkRadius;
			int RelativeZ = ChunkZ - ChunkLoadingCenterZ + HalfChunkRadius;

			CachedChunk = m_LoadedChunks.GetNodeData(glm::uvec3(RelativeX, RelativeY, RelativeZ));
			return CachedChunk;
		}
		return NULL;
	}
}

Voxel* World::GetBlock(const int& X, const int& Y, const int& Z)
{
	Chunk* QueriedChunk = GetLoadedChunk(X / (int) Octree<Voxel>::SIZE, Y / (int) Octree<Voxel>::SIZE, Z / (int) Octree<Voxel>::SIZE);
	return QueriedChunk != NULL ? QueriedChunk->GetVoxel(X % Octree<Voxel>::SIZE, Y % Octree<Voxel>::SIZE, Z % Octree<Voxel>::SIZE) : NULL;;
}

void World::AddBlock(const int& X, const int& Y, const int& Z, const unsigned int& BlockID)
{
	Chunk* ChunkToAddTo = GetLoadedChunk(TOCHUNK_COORD(X, Y, Z));
	if (ChunkToAddTo)
	{
		// This gets the local coordinate in the chunks local coordinate
		// system, which ranges from 0 to 31
		int LocalY = Y % Octree<Voxel>::SIZE;
		int LocalX = X % Octree<Voxel>::SIZE;
		int LocalZ = Z % Octree<Voxel>::SIZE;

		Voxel NewVoxel;
		NewVoxel.BlockID = BlockID;

		ChunkToAddTo->SetVoxel(LocalX, LocalY, LocalZ, &NewVoxel, this);
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

		QueriedChunk->SetVoxel(LocalX, LocalY, LocalZ, NULL, this);
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

					Voxel NewVoxel = Voxel();
					NewVoxel.BlockID = BlockID;
					NewVoxel.Parent = Parent;

					if (XCoord == 0 && YCoord == 0 && ZCoord == 0)
					{
						Parent = &NewVoxel;
					}
					QueriedChunk->SetVoxel(LocalX, LocalY, LocalZ, &NewVoxel, this);

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

						QueriedChunk->SetVoxel(LocalX, LocalY, LocalZ, NULL, this);
					}
					++LocalY;
				}
				++LocalZ;
			}
		}
	}
}

Chunk* World::LoadChunk(const int& ChunkX, const int& ChunkY, const int& ChunkZ)
{
	Chunk* Result = new Chunk();
	Result->m_ChunkX = ChunkX;
	Result->m_ChunkY = ChunkY;
	Result->m_ChunkZ = ChunkZ;

	// Make sure that everything is initialized to zero
	memset(Result->m_pVoxels, NULL, 32 * 32 * 32 * sizeof(Voxel));

	// Create render data for the chunk
	Result->m_pChunkRenderData = ChunkRenderer::CreateRenderData(
		glm::vec3(ChunkX * Octree<Voxel>::SIZE, ChunkX * Octree<Voxel>::SIZE, ChunkX * Octree<Voxel>::SIZE),
		Result);

	return Result;
}
