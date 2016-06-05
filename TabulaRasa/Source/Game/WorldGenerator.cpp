#include "WorldGenerator.h"

#include "..\Engine\Noise.h"
#include "..\Game\World.h"
#include "..\Engine\Chunk.h"

float WorldGenerator::g_ScaleFactor = 30.0f;
int WorldGenerator::g_MaxHeatTiers = 50;

BaseTerrain::~BaseTerrain()
{
}

void BaseTerrain::GenerateToChunk(SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition)
{

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
		ChosenBiome->Generate(m_NoiseGenerator, ChunkPosition * Chunk::SIZE);
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

void IBiome::Generate(SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition)
{
	for (int i = 0; i < ArrayCount(m_Features); ++i)
	{
		if (m_Features[i])
		{
			m_Features[i]->GenerateToChunk(NoiseGenerator, WorldPosition);
		}
	}
}
