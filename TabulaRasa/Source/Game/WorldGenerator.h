#pragma once

#include <string>
#include <unordered_map>

#include "glm/common.hpp"
#include "lua5.2/lua.hpp"
#include "LuaBridge/LuaBridge.h"

#define ArrayCount(Array) sizeof(Array) / sizeof(Array[0])

class Chunk;
class World;
class SimplexNoise;
class Script;

class IFeature
{
public:

	IFeature(std::string Name) : m_Name(Name) {};

	virtual ~IFeature() {}

	virtual void GenerateToChunk(Chunk* Chunk, World* WorldObject, SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition) = 0;

	std::string m_Name;
};

class BaseTerrain : public IFeature
{
public:

	virtual ~BaseTerrain() override;

	virtual void GenerateToChunk(Chunk* Chunk, World* WorldObject, SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition) override;
};

class ScriptedFeature : public IFeature
{
public:

	ScriptedFeature(std::string Name);

	virtual void GenerateToChunk(Chunk* Chunk, World* WorldObject, SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition) override;

	luabridge::LuaRef	m_FeatureTable;
	luabridge::LuaRef	m_GenerateFunction;

};

class IBiome
{
public:

	IBiome();

	virtual ~IBiome();

	void AddFeatureGenerator(int Order, std::string FeatureName);

	virtual void Generate(Chunk* Chunk, World* WorldObject, SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition);

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

class BiomeScript : public IBiome
{
public:

	BiomeScript(std::string BiomeName);

	~BiomeScript() override;

private:

	std::string			m_BiomeName;
	luabridge::LuaRef	m_BiomeInfoTable;
};

class WorldGenerator
{
public:
	WorldGenerator(int Seed, World* WorldObject);

	~WorldGenerator();

	static void LoadFeature(std::string Name, IFeature* Feature);

	void LoadFeatures();

	void GenerateChunk(glm::ivec3 ChunkPosition, Chunk* Result);

	void AddBiome(IBiome* Biome);

	static float	g_ScaleFactor;
	static int		g_MaxHeatTiers;
	
	int				m_Seed;
	World*			m_WorldObject;
	SimplexNoise*	m_NoiseGenerator;
	SimplexNoise*	m_HeatmapGenerator;

	IBiome*			m_Biomes[16];
	int				m_NumBiomes;

	static std::unordered_map<std::string, IFeature*> m_LoadedFeatures;
};
