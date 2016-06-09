#include "WorldGenerator.h"

#include "..\Engine\Noise.h"
#include "..\Game\World.h"
#include "..\Engine\Chunk.h"
#include "..\Engine\ScriptEngine.h"

float WorldGenerator::g_ScaleFactor = 30.0f;
int WorldGenerator::g_MaxHeatTiers = 50;

BaseTerrain::~BaseTerrain()
{
}

void BaseTerrain::GenerateToChunk(Chunk* Chunk, World* WorldObject, SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition)
{
	for (int i = 0; i < Chunk::SIZE; ++i)
	{
		for (int j = 0; j < Chunk::SIZE; ++j)
		{
			int Y = (int)(NoiseGenerator->Noise((WorldPosition.x + i) / 50.0f, (WorldPosition.z + j) / 50.0f) * 10.0f);
			for (int k = 0; k < Chunk::SIZE; ++k)
			{
				if ((WorldPosition.y + k) <= Y)
				{
					Chunk->SetVoxel(WorldObject, i, k, j, 1);
				}
				if (WorldPosition.y + k == 0)
				{
					Chunk->SetVoxel(WorldObject, i, k, j, 1);
				}
			}
		}
	}
}

WorldGenerator::WorldGenerator(int Seed, World* WorldObject) : 
	m_Seed(Seed),
	m_WorldObject(WorldObject),
	m_NoiseGenerator(new SimplexNoise(Seed)),
	m_HeatmapGenerator(new SimplexNoise(Seed * 1337)),
	m_NumBiomes(0)
{
}

WorldGenerator::~WorldGenerator()
{
	for (int i = 0; i < m_NumBiomes; ++i)
	{
		delete m_Biomes[i];
	}

	delete m_NoiseGenerator;
	delete m_HeatmapGenerator;
}

void WorldGenerator::GenerateChunk(glm::ivec3 ChunkPosition, Chunk* Result)
{
	int Heat = (int)(m_HeatmapGenerator->Noise(ChunkPosition.x / g_ScaleFactor, ChunkPosition.z / g_ScaleFactor) * g_MaxHeatTiers);	
	int Height = ChunkPosition.y * Chunk::SIZE;

	IBiome* ChosenBiome = NULL;
	for (unsigned int i = 0; i < m_NumBiomes; ++i)
	{
		IBiome* CurrentBiome = m_Biomes[i];
		if (Heat >= CurrentBiome->m_MinHeatLevel && Heat <= CurrentBiome->m_MaxHeatLevel)
		{
			if (Height >= CurrentBiome->m_MinHeight && Height <= CurrentBiome->m_MaxHeight)
			{
				ChosenBiome = CurrentBiome;
			}
		}
	}

	if (ChosenBiome == NULL)
	{
		// TODO: What should we do if we can't find a biome?
		return;
	}
	else
	{
		ChosenBiome->Generate(Result, m_WorldObject, m_NoiseGenerator, ChunkPosition * Chunk::SIZE);
	}
}

void WorldGenerator::AddBiome(IBiome* Biome)
{
	// Make sure we don't go out of bounds
	assert(m_NumBiomes < ArrayCount(m_Biomes));

	// Insert into next available slot
	m_Biomes[m_NumBiomes++] = Biome;
}

IBiome::IBiome()
{
	for (int i = 0; i < ArrayCount(m_Features); ++i)
	{
		m_Features[i] = NULL;
	}
}

IBiome::~IBiome()
{
	for (int i = 0; i < ArrayCount(m_Features); ++i)
	{
		if (m_Features[i])
		{
			delete m_Features[i];
		}
	}
}

void IBiome::AddFeatureGenerator(int Order, IFeature* Generator)
{
	// Insert at the specified order
	m_Features[Order] = Generator;
}

void IBiome::Generate(Chunk* Chunk, World* WorldObject, SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition)
{
	for (int i = 0; i < ArrayCount(m_Features); ++i)
	{
		if (m_Features[i])
		{
			m_Features[i]->GenerateToChunk(Chunk, WorldObject, NoiseGenerator, WorldPosition);
		}
	}
}

BiomeGrasslands::BiomeGrasslands(int MinHeat, int MaxHeat, int MinHeight, int MaxHeight)
{
	IBiome::IBiome();

	m_MinHeatLevel = MinHeat;
	m_MaxHeatLevel = MaxHeat;
	m_MinHeight = MinHeight;
	m_MaxHeight = MaxHeight;

	AddFeatureGenerator(0, new BaseTerrain());
}

BiomeScript::BiomeScript(const std::string& BiomeName, const std::string& ScriptFilePath) :
	m_BiomeName(BiomeName),
	m_BiomeInfoTable(Script::g_State), // TODO: Fix this since it gets assigned anyways later (below)
	m_GenerateFunction(Script::g_State)
{
	m_pScript = new Script((char*)ScriptFilePath.c_str());
	m_BiomeInfoTable = m_pScript->GetReference(m_BiomeName.c_str());

	// Set the values for the biome picking part of the WorldGenerator
	m_MinHeatLevel = m_BiomeInfoTable["min_heat"];
	m_MaxHeatLevel = m_BiomeInfoTable["max_heat"];
	m_MinHeight = m_BiomeInfoTable["min_height"];
	m_MaxHeight = m_BiomeInfoTable["max_height"];

	// Get the reference to the generate function
	m_GenerateFunction = m_BiomeInfoTable["generate"];
}

BiomeScript::~BiomeScript()
{
	delete m_pScript;
}

void BiomeScript::Generate(Chunk* Chunk, World* WorldObject, SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition)
{
	m_GenerateFunction();
}
