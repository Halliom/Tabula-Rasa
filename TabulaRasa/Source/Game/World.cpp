#include "World.h"

#include "glm\common.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "../Rendering/ChunkRenderComponent.h"

#define CHUNK_LOADING_RADIUS 4

World::World()
{
	CurrentPlayer = new Player();

	TextRender::Initialize2DTextRendering();
	ChunkRenderer::SetupChunkRenderer();

	ChunkLoadingCenterX = 0;
	ChunkLoadingCenterY = 0;
	ChunkLoadingCenterZ = 0;

	LoadedChunks = new Chunk*[CHUNK_LOADING_RADIUS * CHUNK_LOADING_RADIUS * CHUNK_LOADING_RADIUS];
	for (int i = 0; i < CHUNK_LOADING_RADIUS; ++i)
	{
		for (int j = 0; j < CHUNK_LOADING_RADIUS; ++j)
		{
			for (int k = 0; k < CHUNK_LOADING_RADIUS; ++k)
			{
				//TODO: This might be thrashing the cache a bit
				int Index = i + CHUNK_LOADING_RADIUS * (j + CHUNK_LOADING_RADIUS * k);
				int HalfChunkRadius = CHUNK_LOADING_RADIUS / 2;
				LoadedChunks[Index] = LoadChunk(ChunkLoadingCenterX + i - HalfChunkRadius, 
												ChunkLoadingCenterY + j - HalfChunkRadius, 
												ChunkLoadingCenterZ + k - HalfChunkRadius);
			}
		}
	}
	NumLoadedChunks = CHUNK_LOADING_RADIUS * CHUNK_LOADING_RADIUS * CHUNK_LOADING_RADIUS;

	AddBlock(33, 1, 1, 0);

	NumLoadedChunks = 0;
}

World::~World()
{
	TextRender::Destroy2DTextRendering();

	ChunkRenderer::DestroyChunkRenderer();

	if (CurrentPlayer)
	{
		delete CurrentPlayer;
	}

	for (int i = 0; i < NumLoadedChunks; ++i)
	{
		delete LoadedChunks[i];
	}
	delete[] LoadedChunks;
}

void World::Update(float DeltaTime)
{
	static float Angle = 0;
	Angle += DeltaTime;

	ChunkRenderer::RenderAllChunks(CurrentPlayer, Angle);
	TextRender::Render();

	CurrentPlayer->Update(DeltaTime);
	
	for (unsigned int i = 0; i < NumLoadedChunks; ++i)
	{
		//TODO: Change this to be if the dirty flag is set then
		// update the render data and add to multirender draw
		LoadedChunks[i]->Update();
	}
}

void World::AddBlock(const int& X, const int& Y, const int& Z, const unsigned int& BlockID)
{
	int ChunkX = X / Chunk::SIZE;
	int ChunkY = Y / Chunk::SIZE;
	int ChunkZ = Z / Chunk::SIZE;

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
		int LocalX = (X % Chunk::SIZE + Chunk::SIZE) % Chunk::SIZE;
		int LocalY = (Y % Chunk::SIZE + Chunk::SIZE) % Chunk::SIZE;
		int LocalZ = (Z % Chunk::SIZE + Chunk::SIZE) % Chunk::SIZE;

		int Index = RelativeX + CHUNK_LOADING_RADIUS * (RelativeY + CHUNK_LOADING_RADIUS * RelativeZ);
		Chunk* Chunk = LoadedChunks[Index];

		Voxel* NewVoxel = new Voxel();
		NewVoxel->BlockID = BlockID;
		Chunk->InsertVoxel(glm::uvec3(LocalX, LocalY, LocalZ), NewVoxel);
	}
}

void World::RemoveBlock(const int & X, const int & Y, const int & Z)
{
	int ChunkX = X / Chunk::SIZE;
	int ChunkY = Y / Chunk::SIZE;
	int ChunkZ = Z / Chunk::SIZE;


}

Chunk * World::LoadChunk(const int & ChunkX, const int & ChunkY, const int & ChunkZ)
{
	Chunk* Result = new Chunk();
	Result->ChunkX = ChunkX;
	Result->ChunkY = ChunkY;
	Result->ChunkZ = ChunkZ;
	return Result;
}
