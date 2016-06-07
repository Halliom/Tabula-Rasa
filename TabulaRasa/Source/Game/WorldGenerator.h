#pragma once

#include "glm\common.hpp"

#define ArrayCount(Array) sizeof(Array) / sizeof(Array[0])

class IFeature
{
public:

	IFeature() {};

	virtual ~IFeature() {}

	virtual void GenerateToChunk(class Chunk* Chunk, class World* WorldObject, class SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition) = 0;

};

class BaseTerrain : public IFeature
{
public:

	virtual ~BaseTerrain() override;

	virtual void GenerateToChunk(class Chunk* Chunk, class World* WorldObject, class SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition) override;
};

class IBiome
{
public:

	IBiome();

	virtual ~IBiome();

	void AddFeatureGenerator(int Order, IFeature* Generator);

	virtual void Generate(class Chunk* Chunk, class World* WorldObject, class SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition);

	IFeature*	m_Features[8];

	int			m_MinHeatLevel;
	int			m_MaxHeatLevel;
	int			m_MinHeight;
	int			m_MaxHeight;
};

class BiomeGrasslands : public IBiome
{
public:

	BiomeGrasslands(int MinHeat, int MaxHeat, int MinHeight, int MaxHeight);

};

class WorldGenerator
{
public:
	WorldGenerator(int Seed, class World* WorldObject);

	~WorldGenerator();

	void GenerateChunk(glm::ivec3 ChunkPosition, class Chunk* Result);

	void AddBiome(IBiome* Biome);

	static float			g_ScaleFactor;
	static int				g_MaxHeatTiers;
	
	int						m_Seed;
	class World*			m_WorldObject;
	class SimplexNoise*		m_NoiseGenerator;
	class SimplexNoise*		m_HeatmapGenerator;

	IBiome*					m_Biomes[16];
	int						m_NumBiomes;
};
