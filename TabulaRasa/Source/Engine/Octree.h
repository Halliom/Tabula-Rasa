#pragma once

#include <unordered_map>

#include <vector>
#include <deque>
#include <stdint.h>

#include "glm\common.hpp"
#include "DynamicArray.cpp"

#include "../Rendering/ChunkRenderer.h"

class Chunk;

enum VoxelSide : uint32_t
{
	SIDE_EAST = 1,
	SIDE_WEST = 2,
	SIDE_TOP = 4,
	SIDE_BOTTOM = 8,
	SIDE_NORTH = 16,
	SIDE_SOUTH = 32
};

__forceinline int SideToInt(const VoxelSide& Side)
{
	switch (Side)
	{
	case SIDE_EAST: { return 0; break; }
	case SIDE_WEST: { return 1; break; }
	case SIDE_TOP: { return 2; break; }
	case SIDE_BOTTOM: { return 3; break; }
	case SIDE_NORTH: { return 4; break; }
	case SIDE_SOUTH: { return 5; break; }
	}
	return 0; // This won't ever be called but is there to comfort the compiler
}

class Voxel
{
public:

	Voxel() : Chunk(NULL), SidesToRender(63)
	{
	}

	static constexpr float CUBE_SIZE = 1.0f;

	void OnNodeUpdatedAdjacent(const uint8_t& X, const uint8_t& Y, const uint8_t& Z, Voxel* NodeEast, Voxel* NodeWest, Voxel* NodeTop, Voxel* NodeBottom, Voxel* NodeNorth, Voxel* NodeSouth, const bool& Placed);

	void OnNodeUpdatedOnSide(const uint8_t& X, const uint8_t& Y, const uint8_t& Z, const VoxelSide& Side, const bool& Placed);

	uint8_t SidesToRender;
	
	unsigned int BlockID;

	Chunk* Chunk;
};

struct VoxelAddData
{
	glm::uvec3 Position;
	Voxel* Value;
};

class OctreeNode
{
public:
	OctreeNode();

	~OctreeNode();

	size_t GetNodeDepth();

	size_t GetOctantForPosition(const glm::uvec3& Position);

	Voxel* NodeData;

	uint32_t Location;

	uint32_t Size;

	uint8_t Children;

	glm::uvec3 ChunkPosition;

	glm::uvec3 Center;
};

class Chunk
{
public:
	Chunk();

	~Chunk();

	__forceinline void InsertVoxel(glm::uvec3 Position, Voxel* VoxelToAdd)
	{
		ElementsToAdd.Push({ Position, VoxelToAdd });
		ContainsElementsToAdd = true;
	}

	__forceinline void RemoveVoxel(glm::uvec3 Position)
	{
		ContainsElementsToRemove = true;
		ElementsToRemove.Push(Position);
	}

	OctreeNode* GetNode(uint32_t Position);

	OctreeNode* GetNode(const glm::uvec3& Position, OctreeNode* Node);

	OctreeNode* GetNode(const glm::uvec3& Position)
	{
		return GetNode(Position, RootNode);
	}

	void RemoveNode(const glm::uvec3& Position, OctreeNode* Node, bool IsUpwardsRecursive = false);


	void InsertNode(const glm::uvec3& Position, Voxel* NewVoxel, OctreeNode* Node, bool IsNew = true);

	inline Voxel* GetNodeData(const glm::uvec3& Position)
	{
		OctreeNode* Node = GetNode(Position);
		return Node ? Node->NodeData : NULL;
	}

	void Update();

	static constexpr uint32_t SIZE = 32; // Related to DEPTH below

	int ChunkX;

	int ChunkY;

	int ChunkZ;

private:

	static constexpr uint32_t DEPTH = 5; // Chunk size is 2^5=32

	OctreeNode* RootNode;

	glm::uvec3 LastPlacedParentNodeBoxMin;

	glm::uvec3 LastPlacedParentNodeBoxMax;

	OctreeNode* LastPlacedParentNode;

	glm::uvec3 Extent;

	std::unordered_map<uint32_t, OctreeNode*> Nodes;

	DynamicArray<VoxelAddData> ElementsToAdd;

	DynamicArray<glm::uvec3> ElementsToRemove;

	bool ContainsElementsToAdd;

	bool ContainsElementsToRemove;

	bool IsRenderStateDirty;

	ChunkRenderData* RenderData;

private:

	static glm::uvec3 VS_EAST_OFFSET;
	static glm::uvec3 VS_WEST_OFFSET;
	static glm::uvec3 VS_TOP_OFFSET;
	static glm::uvec3 VS_BOTTOM_OFFSET;
	static glm::uvec3 VS_NORTH_OFFSET;
	static glm::uvec3 VS_SOUTH_OFFSET;
};
