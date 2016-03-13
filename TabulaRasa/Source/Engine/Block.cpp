#include "Block.h"

BlockInfo BlockManager::LoadedBlocks[MAX_NUM_BLOCKS];

#define ADD_SOLID(Id, TextureEast, TextureWest, TextureTop, TextureBottom, TextureNorth, TextureSouth) \
		{Id, {TextureEast, TextureWest, TextureTop, TextureBottom, TextureNorth, TextureSouth}, TYPE_SOLID}

void BlockManager::SetupBlocks()
{
	LoadedBlocks[BLOCK_ID_GRASS] = ADD_SOLID(BLOCK_ID_GRASS, 0, 0, 0, 0, 0, 0);
	LoadedBlocks[BLOCK_ID_DIRT] = ADD_SOLID(BLOCK_ID_DIRT, 0, 0, 0, 0, 0, 0);
}
