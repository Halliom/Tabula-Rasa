#include "World.h"

#include "glm\common.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "../Engine/Block.h"
#include "../Engine/PerlinNoise.h"

#define CHUNK_LOADING_RADIUS 4

World::World()
{
	CurrentPlayer = NULL;
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

	PerlinNoise NoiseFunction;

#if 0
	for (int i = -64; i < 64; ++i)
	{
		for (int k = -64; k < 64; ++k)
		{
			int Height = NoiseFunction.GetHeight(i, k);
			for (int j = 0; j < Height; ++j)
			{
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

	AddBlock(1, 1, 1, BLOCK_ID_GRASS);
#endif
}

void World::Update(float DeltaTime)
{
	CurrentPlayer->Update(DeltaTime);
	
	// TODO: Update all the chunks
}

Voxel* World::GetBlock(const int& X, const int& Y, const int& Z)
{
	int ChunkX = X / (int)Octree<Voxel>::SIZE;
	int ChunkY = Y / (int)Octree<Voxel>::SIZE;
	int ChunkZ = Z / (int)Octree<Voxel>::SIZE;

	int HalfChunkRadius = CHUNK_LOADING_RADIUS / 2;
	if ((ChunkX >= ChunkLoadingCenterX - HalfChunkRadius && ChunkX < ChunkLoadingCenterX + HalfChunkRadius) &&
		(ChunkY >= ChunkLoadingCenterY - HalfChunkRadius && ChunkY < ChunkLoadingCenterY + HalfChunkRadius) &&
		(ChunkZ >= ChunkLoadingCenterZ - HalfChunkRadius && ChunkZ < ChunkLoadingCenterZ + HalfChunkRadius))
	{
		// Do the inverse of what we do when adding/creating chunks
		int RelativeX = ChunkX - ChunkLoadingCenterX + HalfChunkRadius;
		int RelativeY = ChunkY - ChunkLoadingCenterY + HalfChunkRadius;
		int RelativeZ = ChunkZ - ChunkLoadingCenterZ + HalfChunkRadius;

		// This gets the local coordinate in the chunks local coordinate 
		// system, which ranges from 0 to 31
		int LocalX = (X % Octree<Voxel>::SIZE + Octree<Voxel>::SIZE) % Octree<Voxel>::SIZE;
		int LocalY = (Y % Octree<Voxel>::SIZE + Octree<Voxel>::SIZE) % Octree<Voxel>::SIZE;
		int LocalZ = (Z % Octree<Voxel>::SIZE + Octree<Voxel>::SIZE) % Octree<Voxel>::SIZE;

		int Index = RelativeX + CHUNK_LOADING_RADIUS * (RelativeY + CHUNK_LOADING_RADIUS * RelativeZ);
		Chunk* QueriedChunk = m_LoadedChunks.GetNodeData(glm::uvec3(RelativeX, RelativeY, RelativeZ));
		
		return QueriedChunk->GetVoxel(LocalX, LocalY, LocalZ);
	}
	return NULL;
}

void World::AddBlock(const int& X, const int& Y, const int& Z, const unsigned int& BlockID)
{
	int ChunkX = X / (int) Octree<Voxel>::SIZE;
	int ChunkY = Y / (int) Octree<Voxel>::SIZE;
	int ChunkZ = Z / (int) Octree<Voxel>::SIZE;

	int HalfChunkRadius = CHUNK_LOADING_RADIUS / 2;
	if ((ChunkX >= ChunkLoadingCenterX - HalfChunkRadius && ChunkX < ChunkLoadingCenterX + HalfChunkRadius) &&
		(ChunkY >= ChunkLoadingCenterY - HalfChunkRadius && ChunkY < ChunkLoadingCenterY + HalfChunkRadius) &&
		(ChunkZ >= ChunkLoadingCenterZ - HalfChunkRadius && ChunkZ < ChunkLoadingCenterZ + HalfChunkRadius))
	{
		// Do the inverse of what we do when adding/creating chunks
		int RelativeX = ChunkX - ChunkLoadingCenterX + HalfChunkRadius;
		int RelativeY = ChunkY - ChunkLoadingCenterY + HalfChunkRadius;
		int RelativeZ = ChunkZ - ChunkLoadingCenterZ + HalfChunkRadius;

		// This gets the local coordinate in the chunks local coordinate 
		// system, which ranges from 0 to 31
		int LocalX = (X % Octree<Voxel>::SIZE + Octree<Voxel>::SIZE) % Octree<Voxel>::SIZE;
		int LocalY = (Y % Octree<Voxel>::SIZE + Octree<Voxel>::SIZE) % Octree<Voxel>::SIZE;
		int LocalZ = (Z % Octree<Voxel>::SIZE + Octree<Voxel>::SIZE) % Octree<Voxel>::SIZE;

		int Index = RelativeX + CHUNK_LOADING_RADIUS * (RelativeY + CHUNK_LOADING_RADIUS * RelativeZ);
		Chunk* ChunkToAddTo = m_LoadedChunks.GetNodeData(glm::uvec3(RelativeX, RelativeY, RelativeZ));

		Voxel* NewVoxel = new Voxel();
		NewVoxel->BlockID = BlockID;

		ChunkToAddTo->SetVoxel(LocalX, LocalY, LocalZ, NewVoxel, this);
	}
}

void World::RemoveBlock(const int & X, const int & Y, const int & Z)
{
	int ChunkX = X / (int)Octree<Voxel>::SIZE;
	int ChunkY = Y / (int)Octree<Voxel>::SIZE;
	int ChunkZ = Z / (int)Octree<Voxel>::SIZE;

	int HalfChunkRadius = CHUNK_LOADING_RADIUS / 2;
	if ((ChunkX >= ChunkLoadingCenterX - HalfChunkRadius && ChunkX < ChunkLoadingCenterX + HalfChunkRadius) &&
		(ChunkY >= ChunkLoadingCenterY - HalfChunkRadius && ChunkY < ChunkLoadingCenterY + HalfChunkRadius) &&
		(ChunkZ >= ChunkLoadingCenterZ - HalfChunkRadius && ChunkZ < ChunkLoadingCenterZ + HalfChunkRadius))
	{
		// Do the inverse of what we do when adding/creating chunks
		int RelativeX = ChunkX - ChunkLoadingCenterX + HalfChunkRadius;
		int RelativeY = ChunkY - ChunkLoadingCenterY + HalfChunkRadius;
		int RelativeZ = ChunkZ - ChunkLoadingCenterZ + HalfChunkRadius;

		// This gets the local coordinate in the chunks local coordinate 
		// system, which ranges from 0 to 31
		int LocalX = (X % Octree<Voxel>::SIZE + Octree<Voxel>::SIZE) % Octree<Voxel>::SIZE;
		int LocalY = (Y % Octree<Voxel>::SIZE + Octree<Voxel>::SIZE) % Octree<Voxel>::SIZE;
		int LocalZ = (Z % Octree<Voxel>::SIZE + Octree<Voxel>::SIZE) % Octree<Voxel>::SIZE;

		int Index = RelativeX + CHUNK_LOADING_RADIUS * (RelativeY + CHUNK_LOADING_RADIUS * RelativeZ);
		Chunk* QueriedChunk = m_LoadedChunks.GetNodeData(glm::uvec3(RelativeX, RelativeY, RelativeZ));

		QueriedChunk->SetVoxel(LocalX, LocalY, LocalZ, NULL, this);
	}
}

Chunk * World::LoadChunk(const int& ChunkX, const int& ChunkY, const int& ChunkZ)
{
	Chunk* Result = new Chunk();
	Result->m_ChunkX = ChunkX;
	Result->m_ChunkY = ChunkY;
	Result->m_ChunkZ = ChunkZ;
	return Result;
}
