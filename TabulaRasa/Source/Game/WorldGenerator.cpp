#include "WorldGenerator.h"

#include "..\Engine\Noise.h"
#include "..\Game\World.h"

BaseTerrain::~BaseTerrain()
{
}

void BaseTerrain::GenerateToChunk(SimplexNoise* NoiseGenerator)
{

}

WorldGenerator::WorldGenerator(int Seed, World* WorldObject) : 
	m_Seed(Seed),
	m_WorldObject(WorldObject),
	m_NoiseGenerator(new SimplexNoise(Seed)),
	m_HeatmapGenerator(new SimplexNoise(Seed * 1337))
{
}

WorldGenerator::~WorldGenerator()
{
	delete m_NoiseGenerator;
}

void WorldGenerator::GenerateChunk(glm::ivec2 ChunkPosition)
{
}

ChunkGenerator::ChunkGenerator() : 
	m_NumFeatures(0)
{
}

ChunkGenerator::~ChunkGenerator()
{
	for (int i = 0; i < m_NumFeatures; ++i)
	{
		delete m_Features[i];
	}
}

void ChunkGenerator::AddFeatureGenerator(IFeature* Generator)
{
	// Make sure that we never have more than the maximum amount
	assert(m_NumFeatures < 8);
	// Insert the Generator into the list
	m_Features[m_NumFeatures++] = Generator;
}
