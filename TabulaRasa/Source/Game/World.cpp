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

FORCEINLINE unsigned int ChunkMod(const int& Value)
{
	static constexpr int ModuloMinusOne = Chunk::SIZE - 1;
	return Value >= 0 ? Value % Chunk::SIZE : ((Value + 1) % Chunk::SIZE) + ModuloMinusOne;
}

Voxel* World::GetBlock(const int& X, const int& Y, const int& Z)
{
	Chunk* QueriedChunk = GetLoadedChunk(TOCHUNK_COORD(X, Y, Z));
    return QueriedChunk != NULL ? QueriedChunk->GetVoxel(ChunkMod(X), ChunkMod(Y), ChunkMod(Z)) : NULL;;
}

void World::AddBlock(const int& X, const int& Y, const int& Z, const unsigned int& BlockID)
{
	assert(BlockID != 0);

	Chunk* ChunkToAddTo = GetLoadedChunk(TOCHUNK_COORD(X, Y, Z));
	if (ChunkToAddTo)
	{
		// This gets the local coordinate in the chunks local coordinate
		// system, which ranges from 0 to 31
		int LocalX = ChunkMod(X);
		int LocalY = ChunkMod(Y);
		int LocalZ = ChunkMod(Z);

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
		int LocalX = ChunkMod(X);
		int LocalY = ChunkMod(Y);
		int LocalZ = ChunkMod(Z);

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
		int LocalX = ChunkMod(X);
		int LocalY = ChunkMod(Y);
		int LocalZ = ChunkMod(Z);

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
		int LocalX = ChunkMod(X);
		int LocalY = ChunkMod(Y);
		int LocalZ = ChunkMod(Z);

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
						LocalX = ChunkMod(LocalX);
					}
					if (LocalY >= 16)
					{
						++ChunkY;
						QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
						LocalY = ChunkMod(LocalY);
					}
					if (LocalZ >= 16)
					{
						++ChunkZ;
						QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
						LocalZ = ChunkMod(LocalZ);
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
							LocalX = ChunkMod(LocalX);
						}
						if (LocalY >= 16)
						{
							++ChunkY;
							QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
							LocalY = ChunkMod(LocalY);
						}
						if (LocalZ >= 16)
						{
							++ChunkZ;
							QueriedChunk =  GetLoadedChunk(ChunkX, ChunkY, ChunkZ);
							LocalZ = ChunkMod(LocalZ);
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

FORCEINLINE float Floor(float Value)
{
	return Value >= 0 ? glm::floor(Value) : -1 * glm::floor(glm::abs(Value));
}

RayHitResult World::RayTraceVoxels(const Ray& Ray)
{
	RayHitResult Result;

	if (Ray.Direction.x == 0 && Ray.Direction.y == 0 && Ray.Direction.z == 0)
	{
		return Result;
	}

	glm::vec3 CameraPosition = Ray.Origin;

	// The voxel we're starting in
	float PositionX = CameraPosition.x >= 0 ? Floor(CameraPosition.x) : Floor(CameraPosition.x - 1.0f);
	float PositionY = CameraPosition.y >= 0 ? Floor(CameraPosition.y) : Floor(CameraPosition.y - 1.0f);
	float PositionZ = CameraPosition.z >= 0 ? Floor(CameraPosition.z) : Floor(CameraPosition.z - 1.0f);

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
		(float)stepZ / Ray.Direction.z);

	// What face we entered through
	int FaceX = 0;
	int FaceY = 0;
	int FaceZ = 0;
    
	do 
	{
		Voxel* Hit = GetBlock(PositionX, PositionY, PositionZ);
		if (Hit && Hit->BlockID != 0) 
		{
			RemoveBlock(PositionX, PositionY, PositionZ);
			Result.Position = glm::ivec3(PositionX, PositionY, PositionZ);
			Result.Side = SideHelper::SideFromDirection(FaceX, FaceY, FaceZ);
			break;
		}
		if (tMax.x < tMax.y) 
		{
			if (tMax.x < tMax.z) 
			{
				if (tMax.x > Ray.Distance) 
					break;

				PositionX += stepX;
				tMax[0] += tDelta[0];

				FaceX = -stepX;
				FaceY = 0;
				FaceZ = 0;
			}
			else
            {
				if (tMax.z > Ray.Distance) 
					break;

				PositionZ += stepZ;
				tMax.z += tDelta.z;

				FaceX = 0;
				FaceY = 0;
				FaceZ = -stepZ;
			}
		}
		else 
		{
			if (tMax.y < tMax.z)
			{
				if (tMax.y > Ray.Distance)
					break;

				PositionY += stepY;
				tMax.y += tDelta.y;

				FaceX = 0;
				FaceY = -stepY;
				FaceZ = 0;
			}
			else 
			{
				if (tMax.z > Ray.Distance) 
					break;

				PositionZ += stepZ;
				tMax.z += tDelta.z;

				FaceX = 0;
				FaceY = 0;
				FaceZ = -stepZ;
			}
		}
	} while (true);

    LogF("(%d, %d, %d)", stepX, stepY, stepZ);
    
	return Result;
}