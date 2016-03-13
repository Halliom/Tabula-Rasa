#include "Chunk.h"

#include "../Game/World.h"

void Chunk::SetVoxel(unsigned int X, unsigned int Y, unsigned int Z, Voxel* NewVoxel, World* WorldObject)
{
	m_pVoxels[X][Y][Z] = *NewVoxel;

	NewVoxel->OnNodeUpdatedAdjacent(
		(uint8_t)X,
		(uint8_t)Y,
		(uint8_t)Z,
		WorldObject->GetBlock(X + 1, Y, Z),
		WorldObject->GetBlock(X - 1, Y, Z),
		WorldObject->GetBlock(X, Y + 1, Z),
		WorldObject->GetBlock(X, Y - 1, Z),
		WorldObject->GetBlock(X, Y, Z + 1),
		WorldObject->GetBlock(X, Y, Z - 1),
		NewVoxel == NULL ? false : true // If we are adding/placing a block, set Placed to true, else set it to false
		);

	m_bIsRenderStateDirty = true;
}
