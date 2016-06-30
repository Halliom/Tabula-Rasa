#include "Block.h"

BlockInfo BlockManager::LoadedBlocks[MAX_NUM_BLOCKS];

#define ADD_SOLID(Id, TextureEast, TextureWest, TextureTop, TextureBottom, TextureNorth, TextureSouth) \
		LoadedBlocks[Id] = BlockInfo(Id, TYPE_SOLID, TextureEast, TextureWest, TextureTop, TextureBottom, TextureNorth, TextureSouth)
#define ADD_MULTIBLOCK(Id, Width, Height, Depth) \
		LoadedBlocks[Id] = BlockInfo(Id, TYPE_MULTIBLOCK, Width, Height, Depth)

void BlockManager::SetupBlocks()
{
	ADD_SOLID(BLOCK_ID_GRASS, 1, 1, 0, 1, 1, 1);
	ADD_SOLID(BLOCK_ID_DIRT, 1, 1, 1, 1, 1, 1);
	ADD_MULTIBLOCK(BLOCK_ID_CHEST, 2, 2, 2);
	ADD_SOLID(BLOCK_ID_REDROCK, 2, 2, 2, 2, 2, 2);
	ADD_SOLID(BLOCK_ID_REDSAND, 3, 3, 3, 3, 3, 3);
}
