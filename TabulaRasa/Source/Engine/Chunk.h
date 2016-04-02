#pragma once

#include "../Game/World.h"
#include "Octree.h"

class Chunk
{
public:

	Voxel m_pVoxels[32][32][32];

	__forceinline Voxel* GetVoxel(unsigned int X, unsigned int Y, unsigned int Z)
	{
		Voxel *Result = &m_pVoxels[X][Y][Z];
		return Result->BlockID > 0 ? Result : NULL; // Only return a value if the block id is not 0
	}

	__forceinline void RemoveVoxel(World *WorldObject, unsigned int X, unsigned int Y, unsigned int Z)
	{
		m_pVoxels[X][Y][Z].BlockID = 0;
		m_pVoxels[X][Y][Z].Parent = NULL;
		m_pVoxels[X][Y][Z].SidesToRender = 0;

		m_pVoxels[X][Y][Z].OnNodeUpdatedAdjacent(
			(uint8_t)X,
			(uint8_t)Y,
			(uint8_t)Z,
			WorldObject->GetBlock(X + 1, Y, Z),
			WorldObject->GetBlock(X - 1, Y, Z),
			WorldObject->GetBlock(X, Y + 1, Z),
			WorldObject->GetBlock(X, Y - 1, Z),
			WorldObject->GetBlock(X, Y, Z + 1),
			WorldObject->GetBlock(X, Y, Z - 1),
			false // If we are adding/placing a block, set Placed to true, else set it to false
			);

		m_bIsRenderStateDirty = true;
	}

	__forceinline void SetVoxel(World *WorldObject, unsigned int X, unsigned int Y, unsigned int Z, unsigned int BlockID, Voxel* Parent = NULL)
	{
		m_pVoxels[X][Y][Z].BlockID = BlockID;
		m_pVoxels[X][Y][Z].Parent = Parent;
		m_pVoxels[X][Y][Z].LocalPosX = X;
		m_pVoxels[X][Y][Z].LocalPosY = Y;
		m_pVoxels[X][Y][Z].LocalPosZ = Z;

		m_pVoxels[X][Y][Z].OnNodeUpdatedAdjacent(
			(uint8_t)X,
			(uint8_t)Y,
			(uint8_t)Z,
			WorldObject->GetBlock(X + 1, Y, Z),
			WorldObject->GetBlock(X - 1, Y, Z),
			WorldObject->GetBlock(X, Y + 1, Z),
			WorldObject->GetBlock(X, Y - 1, Z),
			WorldObject->GetBlock(X, Y, Z + 1),
			WorldObject->GetBlock(X, Y, Z - 1),
			true // If we are adding/placing a block, set Placed to true, else set it to false
			);

		m_bIsRenderStateDirty = true;
	}

	int m_ChunkX;
	int m_ChunkY;
	int m_ChunkZ;

	bool m_bIsRenderStateDirty;
	struct ChunkRenderData* m_pChunkRenderData;
};
