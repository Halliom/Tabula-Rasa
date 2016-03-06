#pragma once

#define MAX_NUM_BLOCKS 4096

#define BLOCK_ID_GRASS 1
#define BLOCK_ID_DIRT 2

enum BlockRenderType
{
	RENDER_TYPE_SOLID,
	RENDER_TYPE_CUSTOM
};

struct BlockInfo
{
	/**
	 * Id of the block, this is used when storing
	 * the block and also goes for blocks that require
	 * custom rendering
	 */
	unsigned int BlockID;

	/**
	 * Texture index for each side
	 */
	unsigned int Textures[6];

	/**
	 * The rendertype for this block, if it should
	 * be rendered normally or if some kind of custom 
	 * rendering should be done
	 */
	BlockRenderType RenderType;
};

class BlockManager
{
public:

	static void SetupBlocks();

	static BlockInfo LoadedBlocks[MAX_NUM_BLOCKS];
};