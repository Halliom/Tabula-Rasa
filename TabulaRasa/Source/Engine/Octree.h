#pragma once

#include <unordered_map>

#include <vector>
#include <deque>
#include <stdint.h>

#include "glm\common.hpp"
#include "DynamicArray.cpp"

class Chunk;
class ChunkRenderer;

enum VoxelSide : uint32_t
{
	SIDE_EAST = 1,
	SIDE_WEST = 2,
	SIDE_TOP = 4,
	SIDE_BOTTOM = 8,
	SIDE_NORTH = 16,
	SIDE_SOUTH = 32
};

struct VoxelVertex
{
	glm::vec3 Position;
	uint8_t ColorRed;
	uint8_t ColorGreen;
	uint8_t ColorBlue;
};

class Voxel
{
public:

	Voxel() : Chunk(NULL), SidesToRender(63)
	{
	}

	static float CUBE_SIZE;

	void OnNodePlacedAdjacent(Voxel* NodeEast, Voxel* NodeWest, Voxel* NodeTop, Voxel* NodeBottom, Voxel* NodeNorth, Voxel* NodeSouth);

	void OnNodePlacedOnSide(const VoxelSide& Side);

	uint8_t SidesToRender;
	uint8_t ColorRed;
	uint8_t ColorGreen;
	uint8_t ColorBlue;

	Chunk* Chunk;
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

struct VoxelAddData
{
	glm::uvec3 Position;
	Voxel* Value;
};

class Chunk
{
public:
	Chunk();

	~Chunk();

	__forceinline void InsertVoxel(glm::uvec3 Position, Voxel* VoxelToAdd)
	{
		ElementsToAdd.push_back({ Position, VoxelToAdd });
		ContainsElementsToAdd = true;
	}

	__forceinline void RemoveVoxel(glm::uvec3 Position)
	{
		ContainsElementsToRemove = true;
		ElementsToRemove.push_back(Position);
	}

	OctreeNode* GetNode(uint32_t Position);

	OctreeNode* GetNode(const glm::uvec3& Position, OctreeNode* Node);

	OctreeNode* GetNode(const glm::uvec3& Position)
	{
		return GetNode(Position, RootNode);
	}

	void RemoveNode(const glm::uvec3& Position, OctreeNode* Node, bool IsUpwardsRecursive = false);

	void RemoveNode(const glm::uvec3& Position)
	{
		RemoveNode(Position, RootNode);
	}

	void InsertNode(const glm::uvec3& Position, Voxel* NewVoxel, OctreeNode* Node, bool IsNew = true);

	void InsertNode(const glm::uvec3& Position, Voxel* NewVoxel)
	{
		InsertNode(Position, NewVoxel, RootNode);
	}

	inline Voxel* GetNodeData(const glm::uvec3& Position)
	{
		OctreeNode* Node = GetNode(Position);
		return Node ? Node->NodeData : NULL;
	}

	void Update();

private:

	static uint32_t DEPTH;

	OctreeNode* RootNode;

	glm::uvec3 LastPlacedParentNodeBoxMin;

	glm::uvec3 LastPlacedParentNodeBoxMax;

	OctreeNode* LastPlacedParentNode;

	glm::uvec3 Extent;

	std::unordered_map<uint32_t, OctreeNode*> Nodes;

	std::deque<VoxelAddData> ElementsToAdd;

	std::deque<glm::uvec3> ElementsToRemove;

	bool ContainsElementsToAdd;

	bool ContainsElementsToRemove;

	bool IsRenderStateDirty;

private:

	static glm::uvec3 VS_EAST_OFFSET;
	static glm::uvec3 VS_WEST_OFFSET;
	static glm::uvec3 VS_TOP_OFFSET;
	static glm::uvec3 VS_BOTTOM_OFFSET;
	static glm::uvec3 VS_NORTH_OFFSET;
	static glm::uvec3 VS_SOUTH_OFFSET;
};

