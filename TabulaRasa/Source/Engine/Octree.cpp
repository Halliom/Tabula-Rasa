#include "Octree.h"

#include <intrin.h>

static glm::uvec3 VS_EAST_OFFSET = glm::uvec3(1, 0, 0);
static glm::uvec3 VS_WEST_OFFSET = glm::uvec3(-1, 0, 0);
static glm::uvec3 VS_TOP_OFFSET = glm::uvec3(0, 1, 0);
static glm::uvec3 VS_BOTTOM_OFFSET = glm::uvec3(0, -1, 0);
static glm::uvec3 VS_NORTH_OFFSET = glm::uvec3(0, 0, 1);
static glm::uvec3 VS_SOUTH_OFFSET = glm::uvec3(0, 0, -1);

void Voxel::OnNodeUpdatedAdjacent(const uint8_t& X, const uint8_t& Y, const uint8_t& Z, Voxel* NodeEast, Voxel* NodeWest, Voxel* NodeTop, Voxel* NodeBottom, Voxel* NodeNorth, Voxel* NodeSouth, const bool& Placed)
{
	uint8_t Result = 63;
	if (NodeEast)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_EAST;
		NodeEast->OnNodeUpdatedOnSide(X + 1, Y, Z, VoxelSide::SIDE_WEST, Placed);
	}
	if (NodeWest)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_WEST;
		NodeWest->OnNodeUpdatedOnSide(X - 1, Y, Z, VoxelSide::SIDE_EAST, Placed);
	}
	if (NodeTop)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_TOP;
		NodeTop->OnNodeUpdatedOnSide(X, Y + 1, Z, VoxelSide::SIDE_BOTTOM, Placed);
	}
	if (NodeBottom)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_BOTTOM;
		NodeBottom->OnNodeUpdatedOnSide(X, Y - 1, Z, VoxelSide::SIDE_TOP, Placed);
	}
	if (NodeNorth)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_NORTH;
		NodeNorth->OnNodeUpdatedOnSide(X, Y, Z + 1, VoxelSide::SIDE_SOUTH, Placed);
	}
	if (NodeSouth)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_SOUTH;
		NodeSouth->OnNodeUpdatedOnSide(X, Y, Z - 1, VoxelSide::SIDE_NORTH, Placed);
	}

	if (!Placed)
	{
		SidesToRender = 0;
	}
	else
	{
		SidesToRender = Result;
	}
}

// Placed means that the original block which called this function was placed not removed
__forceinline void Voxel::OnNodeUpdatedOnSide(const uint8_t& X, const uint8_t& Y, const uint8_t& Z, const VoxelSide& Side, const bool& Placed)
{
	if (Placed)
	{
		// Make sure it does NOT render
		SidesToRender ^= Side;
	}
	else
	{
		// Make sure it DOES render
		SidesToRender |= Side;
	}
}