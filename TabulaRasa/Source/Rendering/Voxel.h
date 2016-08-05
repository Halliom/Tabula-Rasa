#pragma once

#include "../Engine/VoxelSide.h"

class Voxel
{
public:

	Voxel() :
		SidesToRender(63),
		Rotation(0),
		BlockID(0),
		Parent(nullptr)
	{
	}

	Voxel(unsigned short InitBlockID) :
		SidesToRender(63),
		Rotation(0),
		BlockID(InitBlockID),
		Parent(nullptr)
	{
	}

	Voxel(unsigned char InitSidesToRender, unsigned short InitBlockID) :
		SidesToRender(InitSidesToRender),
		Rotation(0),
		BlockID(InitBlockID),
		Parent(nullptr)
	{
	}

	Voxel(unsigned char InitSidesToRender, unsigned char InitRotation, unsigned short InitBlockID) :
		SidesToRender(InitSidesToRender),
		Rotation(InitRotation),
		BlockID(InitBlockID),
		Parent(nullptr)
	{
	}

	void OnNodeUpdatedAdjacent(const uint8_t& X, const uint8_t& Y, const uint8_t& Z, Voxel* NodeEast, Voxel* NodeWest, Voxel* NodeTop, Voxel* NodeBottom, Voxel* NodeNorth, Voxel* NodeSouth, const bool& Placed);

	void OnNodeUpdatedOnSide(const uint8_t& X, const uint8_t& Y, const uint8_t& Z, const VoxelSide& Side, const bool& Placed);

	unsigned char	SidesToRender;	/* 1 byte aligned */
	unsigned char	Rotation;		/* 1 byte aligned */
	unsigned short	BlockID;		/* 2 byte aligned */
	Voxel*			Parent;			/* 4 byte aligned */
};