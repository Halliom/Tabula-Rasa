#pragma once

#include "glm\common.hpp"

class IFeature
{
public:

	IFeature() {};

	virtual ~IFeature() {}

	virtual void GenerateToChunk(class SimplexNoise* NoiseGenerator) = 0;

};

class BaseTerrain : public IFeature
{
public:

	virtual ~BaseTerrain() override;

	virtual void GenerateToChunk(class SimplexNoise* NoiseGenerator) override;
};

class Biome
{

};

class ChunkGenerator
{
public:

	ChunkGenerator();

	~ChunkGenerator();

	void AddFeatureGenerator(IFeature* Generator);

	class IFeature*		m_Features[8];
	int					m_NumFeatures;
};

class WorldGenerator
{
public:
	WorldGenerator(int Seed, class World* WorldObject);

	~WorldGenerator();

	void GenerateChunk(glm::ivec2 ChunkPosition);

	int					m_Seed;
	class World*		m_WorldObject;
	class SimplexNoise* m_NoiseGenerator;
	class SimplexNoise* m_HeatmapGenerator;
};
