#pragma once

#include <unordered_map>

#include <vector>
#include <deque>
#include <stdint.h>

#include "glm\common.hpp"
#include "DynamicArray.cpp"

#include "../Rendering/ChunkRenderComponent.h"

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

struct VoxelVertex
{
	glm::vec3 Position;
	uint8_t ColorRed;
	uint8_t ColorGreen;
	uint8_t ColorBlue;
};

struct VoxelBufferData
{
	std::vector<ChunkRenderCoordinate> EastFaces;
	std::vector<ChunkRenderCoordinate> WestFaces;
	std::vector<ChunkRenderCoordinate> TopFaces;
	std::vector<ChunkRenderCoordinate> BottomFaces;
	std::vector<ChunkRenderCoordinate> NorthFaces;
	std::vector<ChunkRenderCoordinate> SouthFaces;
};

class Voxel
{
public:

	Voxel() : Chunk(NULL), SidesToRender(63)
	{
	}

	static float CUBE_SIZE;

	void OnNodeUpdatedAdjacent(VoxelBufferData* AddData, VoxelBufferData* RemoveData, const uint8_t& X, const uint8_t& Y, const uint8_t& Z, Voxel* NodeEast, Voxel* NodeWest, Voxel* NodeTop, Voxel* NodeBottom, Voxel* NodeNorth, Voxel* NodeSouth, const bool& Placed);

	void OnNodeUpdatedOnSide(VoxelBufferData* AddData, VoxelBufferData* RemoveData, const uint8_t& X, const uint8_t& Y, const uint8_t& Z, const VoxelSide& Side, const bool& Placed);

	uint8_t SidesToRender;
	uint8_t ColorRed;
	uint8_t ColorGreen;
	uint8_t ColorBlue;

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

	void RemoveNode(VoxelBufferData* AddData, VoxelBufferData* RemoveData, const glm::uvec3& Position, OctreeNode* Node, bool IsUpwardsRecursive = false);


	void InsertNode(VoxelBufferData* AddData, VoxelBufferData* RemoveData, const glm::uvec3& Position, Voxel* NewVoxel, OctreeNode* Node, bool IsNew = true);

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
