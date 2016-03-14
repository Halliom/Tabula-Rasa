#pragma once

#include "Octree.h"

class Chunk
{
public:

	Voxel m_pVoxels[32][32][32];

	Voxel* GetVoxel(unsigned int X, unsigned int Y, unsigned int Z)
	{
		return &m_pVoxels[X][Y][Z];
	}

	void SetVoxel(unsigned int X, unsigned int Y, unsigned int Z, Voxel* NewVoxel, class World* WorldObject);

	unsigned int m_ChunkX;
	unsigned int m_ChunkY;
	unsigned int m_ChunkZ;

	bool m_bIsRenderStateDirty;
	struct ChunkRenderData* m_pChunkRenderData;
};