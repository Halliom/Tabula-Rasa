#pragma once

#include "glm\common.hpp"
#include "Block.h"

enum VoxelSide : unsigned int
{
	SIDE_EAST = 1,
	SIDE_WEST = 2,
	SIDE_TOP = 4,
	SIDE_BOTTOM = 8,
	SIDE_NORTH = 16,
	SIDE_SOUTH = 32
};

class SideHelper
{
public:

	/*
	* Converts a direction into a VoxelSide
	*/
	static VoxelSide SideFromDirection(const int& X, const int& Y, const int& Z)
	{
		switch (X)
		{
			case 1: { return SIDE_EAST; break; }
			case -1: { return SIDE_WEST; break; }
		}
		switch (Y)
		{
			case 1: { return SIDE_TOP; break; }
			case -1: { return SIDE_BOTTOM; break; }
		}
		switch (Z)
		{
			case 1: { return SIDE_NORTH; break; }
			case -1: { return SIDE_SOUTH; break; }
		}
		return (VoxelSide)0; // Should never be called
	}

	/*
	* Converts a direction into a VoxelSide
	*/
	static VoxelSide SideFromDirection(const glm::ivec3& Vec)
	{
		switch (Vec.x)
		{
			case 1: { return SIDE_EAST; break; }
			case -1: { return SIDE_WEST; break; }
		}
		switch (Vec.y)
		{
			case 1: { return SIDE_TOP; break; }
			case -1: { return SIDE_BOTTOM; break; }
		}
		switch (Vec.z)
		{
			case 1: { return SIDE_NORTH; break; }
			case -1: { return SIDE_SOUTH; break; }
		}
		return (VoxelSide)0; // Should never be called
	}

	/*
	* Converts a side into a normalized direction vector like
	* a normal to that 'plane'
	*/
	static glm::vec3 DirectionFromSide(const VoxelSide& Side)
	{
		switch (Side)
		{
			case SIDE_EAST: { return glm::vec3(1.0f, 0.0f, 0.0f); break; }
			case SIDE_WEST: { return glm::vec3(-1.0f, 0.0f, 0.0f); break; }
			case SIDE_TOP: { return glm::vec3(0.0f, 1.0f, 0.0f); break; }
			case SIDE_BOTTOM: { return glm::vec3(0.0f, -1.0f, 0.0f); break; }
			case SIDE_NORTH: { return glm::vec3(0.0f, 0.0f, 1.0f); break; }
			case SIDE_SOUTH: { return glm::vec3(0.0f, 0.0f, -1.0f); break; }
		}
		return glm::vec3(); // This won't ever be called but is there to comfort the compiler
	}

	/*
	* Converts a side into an int from 0-5 instead of the 2's complement alternative
	*/
	static int SideToInt(const VoxelSide& Side)
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

	static int GetRotatedTexture(const BlockInfo& Info, const VoxelSide& Side, const unsigned char& Rotation)
	{
		return Info.RenderData.Textures[SideToInt(Side)];
	}

};
