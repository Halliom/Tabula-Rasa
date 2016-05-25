#pragma once

class WorldGenerator
{
public:
	WorldGenerator();

	~WorldGenerator();

	void GenerateChunk();

	int Seed;
};