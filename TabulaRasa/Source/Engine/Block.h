#pragma once

#include <string.h>

#define MAX_NUM_BLOCKS 4096

#define BLOCK_ID_GRASS 1
#define BLOCK_ID_DIRT 2
#define BLOCK_ID_CHEST 3

enum BlockType
{
	TYPE_SOLID,
	TYPE_MULTIBLOCK,
	TYPE_STRUCTURE
};

struct BlockInfo
{
	BlockInfo()	: 
		BlockID(0), RenderType(TYPE_SOLID)
	{
	}

	BlockInfo(unsigned int BlockID, BlockType RenderType, unsigned int Textures[6]) :
		BlockID(BlockID), RenderType(RenderType)
	{
		memcpy(RenderData.Textures, Textures, sizeof(unsigned int) * 6);
	}

	BlockInfo(unsigned int BlockID, BlockType RenderType, unsigned int TextureEast, unsigned int TextureWest, unsigned int TextureTop, unsigned int TextureBottom, unsigned int TextureNorth, unsigned int TextureSouth) :
		BlockID(BlockID), RenderType(RenderType)
	{
		RenderData.Textures[0] = TextureEast;
		RenderData.Textures[1] = TextureWest;
		RenderData.Textures[2] = TextureTop;
		RenderData.Textures[3] = TextureBottom;
		RenderData.Textures[4] = TextureNorth;
		RenderData.Textures[5] = TextureSouth;
	}

	BlockInfo(unsigned int BlockID, BlockType RenderType, unsigned int Width, unsigned int Height, unsigned int Depth) :
		BlockID(BlockID), RenderType(RenderType)
	{
		RenderData.MultiblockRenderData.Width = Width;
		RenderData.MultiblockRenderData.Height = Height;
		RenderData.MultiblockRenderData.Depth = Depth;
	}

	/**
	 * Id of the block, this is used when storing
	 * the block and also goes for blocks that require
	 * custom rendering
	 */
	unsigned int BlockID;

	/**
	* The rendertype for this block, if it should
	* be rendered normally or if some kind of custom
	* rendering should be done
	*/
	BlockType RenderType;

	union
	{
		/**
		* Texture index for each side
		*/
		unsigned int Textures[6];

		struct
		{
			/**
			* The dimensions for the block (only used if multiblock)
			*/
			unsigned int Width;
			unsigned int Height;
			unsigned int Depth;
		} MultiblockRenderData;
	} RenderData;
};

class BlockManager
{
public:

	static void SetupBlocks();

	static BlockInfo LoadedBlocks[MAX_NUM_BLOCKS];
};