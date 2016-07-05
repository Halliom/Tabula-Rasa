#include "World.h"

#include "glm/common.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "../Platform/Platform.h"
#include "../Engine/Block.h"
#include "../Engine/Chunk.h"
#include "../Engine/ScriptEngine.h"
#include "../Engine/ChunkManager.h"
#include "../Engine/Core/Memory.h"
#include "../Engine/Camera.h"
#include "../Engine/Console.h"
#include "../Game/WorldGenerator.h"
#include "../Game/Player.h"

#define TOCHUNK_COORD(X, Y, Z) X >= 0 ? X / Chunk::SIZE : (X / Chunk::SIZE) - 1, Y >= 0 ? Y / Chunk::SIZE : (Y / Chunk::SIZE) - 1, Z >= 0 ? Z / Chunk::SIZE : (Z / Chunk::SIZE) - 1

World::World()
{
	m_pCurrentPlayer = NULL;
	m_pWorldGenerator = NULL;
	CachedChunk = NULL;
}

World::~World()
{
	if (m_pCurrentPlayer)
	{
		delete m_pCurrentPlayer;
	}

	delete m_pWorldGenerator;
	delete m_pChunkManager;
}

void World::Initialize()
{
	BlockManager::SetupBlocks();

	m_pCurrentPlayer = new Player();
	m_pCurrentPlayer->BeginPlay();
	m_pCurrentPlayer->m_pWorldObject = this;

	m_pWorldGenerator = new WorldGenerator(123123, this);
	m_pWorldGenerator->LoadFeatures();

//	m_pWorldGenerator->AddBiome(new BiomeGrasslands(0, 50, -1, 10));

	m_pChunkManager = new ChunkManager(this, m_pWorldGenerator, 1);
	m_pChunkManager->LoadNewChunks(glm::ivec3(0, 0, 0));
}

void World::Update(float DeltaTime)
{
	glm::ivec3 PlayerChunkPosition = m_pCurrentPlayer->m_Position / (float)Chunk::SIZE;
	glm::ivec3 OldPlayerChunkPosition = m_pCurrentPlayer->m_OldPosition / (float)Chunk::SIZE;

	// If the player moved to a new chunk
	if (PlayerChunkPosition != OldPlayerChunkPosition)
	{
		// Unload/load chunks (unload first to make room for new ones)
		m_pChunkManager->UnloadChunks(PlayerChunkPosition);
		m_pChunkManager->LoadNewChunks(PlayerChunkPosition);
	}
    
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

FORCEINLINE unsigned int ChunkMod(const int& Value, const int& Mod)
{
	return Value >= 0 ? Value % Mod : (Value % Mod) + Mod - 1;
}

Voxel* World::GetBlock(const int& X, const int& Y, const int& Z)
{
	Chunk* QueriedChunk = GetLoadedChunk(TOCHUNK_COORD(X, Y, Z));
    return QueriedChunk != NULL ? QueriedChunk->GetVoxel(ChunkMod(X, Chunk::SIZE), ChunkMod(Y, Chunk::SIZE), ChunkMod(Z, Chunk::SIZE)) : NULL;;
}

void World::AddBlock(const int& X, const int& Y, const int& Z, const unsigned int& BlockID)
{
	assert(BlockID != 0);

	Chunk* ChunkToAddTo = GetLoadedChunk(TOCHUNK_COORD(X, Y, Z));
	if (ChunkToAddTo)
	{
		// This gets the local coordinate in the chunks local coordinate
		// system, which ranges from 0 to 31
		int LocalX = ChunkMod(X, Chunk::SIZE);
		int LocalY = ChunkMod(Y, Chunk::SIZE);
		int LocalZ = ChunkMod(Z, Chunk::SIZE);

		ChunkToAddTo->SetVoxel(this, LocalX, LocalY, LocalZ, BlockID);
	}
}

void World::RemoveBlock(const int& X, const int& Y, const int& Z)
{
	Chunk* QueriedChunk = GetLoadedChunk(TOCHUNK_COORD(X, Y, Z));
	if (QueriedChunk)
	{
		// This gets the local coordinate in the chunks local coordinate
		// system, which ranges from 0 to 31
		int LocalX = ChunkMod(X, Chunk::SIZE);
		int LocalY = ChunkMod(Y, Chunk::SIZE);
		int LocalZ = ChunkMod(Z, Chunk::SIZE);

		QueriedChunk->RemoveVoxel(this, LocalX, LocalY, LocalZ);
	}
}

Voxel* World::GetMultiblock(const int &X, const int &Y, const int &Z)
{
	int ChunkX = X >= 0 ? X / Chunk::SIZE : (X / Chunk::SIZE) - 1;
	int ChunkY = Y >= 0 ? Y / Chunk::SIZE : (Y / Chunk::SIZE) - 1;
	int ChunkZ = Z >= 0 ? Z / Chunk::SIZE : (Z / Chunk::SIZE) - 1;

	Chunk* QueriedChunk = GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
	if (QueriedChunk)
	{
		// This gets the local coordinate in the chunks local coordinate
		// system, which ranges from 0 to 31
		int LocalX = ChunkMod(X, Chunk::SIZE);
		int LocalY = ChunkMod(Y, Chunk::SIZE);
		int LocalZ = ChunkMod(Z, Chunk::SIZE);

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

	int ChunkX = X >= 0 ? X / Chunk::SIZE : (X / Chunk::SIZE) - 1;
	int ChunkY = Y >= 0 ? Y / Chunk::SIZE : (Y / Chunk::SIZE) - 1;
	int ChunkZ = Z >= 0 ? Z / Chunk::SIZE : (Z / Chunk::SIZE) - 1;

	Chunk *QueriedChunk = GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
	if (QueriedChunk)
	{
		// This gets the local coordinate in the chunks local coordinate
		// system, which ranges from 0 to 31
		int LocalX = ChunkMod(X, Chunk::SIZE);
		int LocalY = ChunkMod(Y, Chunk::SIZE);
		int LocalZ = ChunkMod(Z, Chunk::SIZE);

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
						LocalX = ChunkMod(LocalX, Chunk::SIZE);
					}
					if (LocalY >= 16)
					{
						++ChunkY;
						QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
						LocalY = ChunkMod(LocalY, Chunk::SIZE);
					}
					if (LocalZ >= 16)
					{
						++ChunkZ;
						QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
						LocalZ = ChunkMod(LocalZ, Chunk::SIZE);
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
		int ChunkX = X >= 0 ? X / Chunk::SIZE : (X / Chunk::SIZE) - 1;
		int ChunkY = Y >= 0 ? Y / Chunk::SIZE : (Y / Chunk::SIZE) - 1;
		int ChunkZ = Z >= 0 ? Z / Chunk::SIZE : (Z / Chunk::SIZE) - 1;

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
							LocalX = ChunkMod(LocalX, Chunk::SIZE);
						}
						if (LocalY >= 16)
						{
							++ChunkY;
							QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
							LocalY = ChunkMod(LocalY, Chunk::SIZE);
						}
						if (LocalZ >= 16)
						{
							++ChunkZ;
							QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
							LocalZ = ChunkMod(LocalZ, Chunk::SIZE);
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

FORCEINLINE float Mod(float Value, float Mod)
{
	return glm::mod((glm::mod(Value, Mod) + Mod), Mod);
}

// Find t where Direction * t + Origin = integer
float GetStep(float Origin, float Direction)
{
	if (Direction < 0)
		return GetStep(-Origin, -Direction);
	else
	{
		float Rem = Mod(Origin, 1.0f);
		return (1.0f - Rem) / Direction;
	}
}

FORCEINLINE float intbound(float s, float ds)
{
	if (ds < 0) {
		return intbound(-s, -ds);
	}
	else {
		s = Mod(s, 1);
		// problem is now s+t*ds = 1
		return (1 - s) / ds;
	}
}

RayHitResult World::RayTraceWorld(const Ray& Ray)
{
	RayHitResult Result;

	if (Ray.Direction.x == 0 && Ray.Direction.y == 0 && Ray.Direction.z == 0)
	{
		return Result;
	}

	glm::vec3 CameraPosition = Ray.Origin;

	// The voxel we're starting in
	float PositionX = glm::floor(CameraPosition.x);
	float PositionY = glm::floor(CameraPosition.y);
	float PositionZ = glm::floor(CameraPosition.z);

	// The direction in which to take a step
	int stepX = Ray.Direction.x > 0 ? 1 : Ray.Direction.x < 0 ? -1 : 0;
	int stepY = Ray.Direction.y > 0 ? 1 : Ray.Direction.y < 0 ? -1 : 0;
	int stepZ = Ray.Direction.z > 0 ? 1 : Ray.Direction.z < 0 ? -1 : 0;

	// How much we need to multiply the Direction vector with to get to an int
	glm::vec3 tMax(
		intbound(CameraPosition.x, Ray.Direction.x), 
		intbound(CameraPosition.y, Ray.Direction.y), 
		intbound(CameraPosition.z, Ray.Direction.z));

	// How much we need to multiply the direction vector with to get to the next int
	glm::vec3 tDelta(
		(float)stepX / Ray.Direction.x, 
		(float)stepY / Ray.Direction.y, 
		(float)stepZ / Ray.Direction.y);

	// What face we entered through
	float FaceX;
	float FaceY;
	float FaceZ;

	do 
	{
		Voxel* Hit = GetBlock(PositionX, PositionY, PositionZ);
		if (Hit && Hit->BlockID != 0) 
		{
			RemoveBlock(PositionX, PositionY, PositionZ);
			break;
		}
		if (tMax[0] < tMax[1]) 
		{
			if (tMax[0] < tMax[2]) 
			{
				if (tMax[0] > Ray.Distance) 
					break;

				PositionX += stepX;
				tMax[0] += tDelta[0];

				FaceX = -stepX;
				FaceY = 0;
				FaceZ = 0;
			}
			else {
				if (tMax[2] > Ray.Distance) 
					break;

				PositionZ += stepZ;
				tMax[2] += tDelta[2];

				FaceX = 0;
				FaceY = 0;
				FaceZ = -stepZ;
			}
		}
		else 
		{
			if (tMax[1] < tMax[2])
			{
				if (tMax[1] > Ray.Distance)
					break;

				PositionY += stepY;
				tMax[1] += tDelta[1];

				FaceX = 0;
				FaceY = -stepY;
				FaceZ = 0;
			}
			else 
			{
				if (tMax[2] > Ray.Distance) 
					break;

				PositionZ += stepZ;
				tMax[2] += tDelta[2];

				FaceX = 0;
				FaceY = 0;
				FaceZ = -stepZ;
			}
		}
	} while (true);

	return Result;
}