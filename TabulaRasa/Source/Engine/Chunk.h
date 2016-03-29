#pragma once

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

	//void RemoveVoxel(unsigned int X, unsigned int Y, unsigned int Z, class World *WorldObject);

	void SetVoxel(unsigned int X, unsigned int Y, unsigned int Z, Voxel* NewVoxel, class World *WorldObject);

	unsigned int m_ChunkX;
	unsigned int m_ChunkY;
	unsigned int m_ChunkZ;

	bool m_bIsRenderStateDirty;
	struct ChunkRenderData* m_pChunkRenderData;
};
