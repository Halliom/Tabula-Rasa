#include "WorldGenerator.h"

#include <Windows.h>

#include "..\Platform\Platform.h"
#include "..\Engine\Noise.h"
#include "..\Game\World.h"
#include "..\Engine\Chunk.h"
#include "..\Engine\ScriptEngine.h"
#include "..\Engine\Console.h"

float WorldGenerator::g_ScaleFactor = 30.0f;
int WorldGenerator::g_MaxHeatTiers = 50;
std::unordered_map<std::string, IFeature*> WorldGenerator::m_LoadedFeatures;

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

	for (auto& KVPair : m_LoadedFeatures)
	{
		delete KVPair.second;
	}

	delete m_NoiseGenerator;
	delete m_HeatmapGenerator;
}

void WorldGenerator::LoadFeature(std::string Name, IFeature* Feature)
{
	// Make sure it's not already loaded
	assert(m_LoadedFeatures.find(Name) == m_LoadedFeatures.end());

	// Inser the feature into the map
	m_LoadedFeatures.insert({ Name, Feature });
}

void WorldGenerator::LoadFeatures()
{
	// TODO: Make platform independent
	WIN32_FIND_DATA FileFindData;
	HANDLE FoundFile = NULL;

	char Path[2048];
	std::string DirectoryCppString = PlatformFileSystem::GetAssetDirectory(DT_SCRIPTS).c_str();
	DirectoryCppString.append("WorldGen\\");
	const char* Directory = DirectoryCppString.c_str();

	//Specify a file mask. *.* = We want everything!
	sprintf(Path, "%s*.*", Directory);

	if ((FoundFile = FindFirstFile(Path, &FileFindData)) == INVALID_HANDLE_VALUE)
	{
		Log("Unable to load World-gen scripts");
	}

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories.
		if (strcmp(FileFindData.cFileName, ".") != 0
			&& strcmp(FileFindData.cFileName, "..") != 0)
		{
			//Build up our file path using the passed in
			//  [sDir] and the file/foldername we just found:
			sprintf(Path, "WorldGen\\%s", FileFindData.cFileName);

			if (FileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// It's a directory, maybe later?
			}
			else 
			{
				// Just create a script object on the stack which will load the
				// contents of the script file into the public Lua state
				Script LoadedScript(Path);
			}
		}
	} while (FindNextFile(FoundFile, &FileFindData)); //Find the next file.

	FindClose(FoundFile); //Always, Always, clean things up!

	// At this point "world_gen.lua" should have been loaded and so we
	// should have access to the table "Biomes" and each one of those
	// biomes should also have been loaded by now

	// Get a reference to the "Biomes" table and push some values onto the stack to
	// be able to iterate through the table
	luabridge::LuaRef BiomesTable = luabridge::getGlobal(Script::g_State, "Biomes");
	BiomesTable.push(Script::g_State);
	luabridge::push(Script::g_State, luabridge::Nil());

	for (luabridge::Iterator It(BiomesTable); !It.isNil(); ++It)
	{
		std::string BiomeName = *It;

		lua_pop(Script::g_State, 1);

		// Add the biome
		BiomeScript* Biome = new BiomeScript(BiomeName);
		AddBiome(Biome);
	}
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
	// Do NOT delete features as they may be used by other biomes
}

void IBiome::AddFeatureGenerator(int Order, std::string FeatureName)
{
	IFeature* Feature = NULL;

	// If it exists in the loaded features list
	if (WorldGenerator::m_LoadedFeatures.find(FeatureName) != WorldGenerator::m_LoadedFeatures.end())
	{
		Feature = WorldGenerator::m_LoadedFeatures.at(FeatureName);
	}
	else
	{
		// Otherwise it might not be loaded just yet, it must be a scripted biome
		luabridge::LuaRef FeatureTable = luabridge::getGlobal(Script::g_State, FeatureName.c_str());
		if (FeatureTable.isTable())
		{
			Feature = new ScriptedFeature(FeatureName);
			WorldGenerator::LoadFeature(FeatureName, Feature);
		}
		else
		{
			LogF("Could not find feature generator %s, it does not exist", FeatureName.c_str());
		}
	}

	// Insert at the specified order
	m_Features[Order] = Feature;
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

//	AddFeatureGenerator(0, "");
}

BiomeScript::BiomeScript(std::string BiomeName) :
	m_BiomeName(BiomeName),
	m_BiomeInfoTable(Script::g_State) // TODO: Fix this since it gets assigned anyways later (below)
{
	m_BiomeInfoTable = luabridge::getGlobal(Script::g_State, m_BiomeName.c_str());

	// Set the values for the biome picking part of the WorldGenerator
	m_MinHeatLevel = m_BiomeInfoTable["min_heat"];
	m_MaxHeatLevel = m_BiomeInfoTable["max_heat"];
	m_MinHeight = m_BiomeInfoTable["min_height"];
	m_MaxHeight = m_BiomeInfoTable["max_height"];

	luabridge::LuaRef Features = m_BiomeInfoTable["features"];

	// No more than 8 features allowed
	assert(Features.length() <= 8);

	// Iterate through all of the features listed
	Features.push(Script::g_State);
	luabridge::push(Script::g_State, luabridge::Nil());
	for (luabridge::Iterator It(Features); !It.isNil(); ++It)
	{
		// Add the feature generator (by its name), note that It.key()
		// is not zero-based, therefore we subtract one off it
		std::string FeatureName = *It;
		int Key = It.key();
		AddFeatureGenerator(Key - 1, FeatureName);
		
		LogF("Biome \"%s\" loaded feature generator \"%s\"", m_BiomeName.c_str(), FeatureName.c_str());
		lua_pop(Script::g_State, 1);
	}
}

BiomeScript::~BiomeScript()
{
}

class SimplexNoiseWrapper
{
public:

	float Noise(float X, float Y) { return m_pNoise->Noise(X, Y); }

	SimplexNoise* m_pNoise;
};

class ChunkWrapper
{
public:

	void SetBlock(int X, int Y, int Z, int BlockID) { m_pChunk->SetVoxel(m_pWorldObject, X, Y, Z, BlockID, NULL); }

	Chunk* m_pChunk;
	World* m_pWorldObject;
};

static bool Initialized = false;

ScriptedFeature::ScriptedFeature(std::string Name) : 
	IFeature(Name),
	m_FeatureTable(Script::g_State),
	m_GenerateFunction(Script::g_State)
{
	m_FeatureTable = luabridge::getGlobal(Script::g_State, Name.c_str());

	m_GenerateFunction = m_FeatureTable["generate"];
}

void ScriptedFeature::GenerateToChunk(Chunk* Chunk, World* WorldObject, SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition)
{
	if (!Initialized)
	{
		luabridge::getGlobalNamespace(Script::g_State)
			.beginClass<SimplexNoiseWrapper>("SimplexNoiseWrapper")
			.addFunction("noise", &SimplexNoiseWrapper::Noise)
			.endClass()
			.beginClass<ChunkWrapper>("ChunkWrapper")
			.addFunction("set_block", &ChunkWrapper::SetBlock)
			.endClass();
		Initialized = true;
	}

	// Prepare the Noise object wrapper
	SimplexNoiseWrapper* NoiseObject = new SimplexNoiseWrapper();
	NoiseObject->m_pNoise = NoiseGenerator;

	// Prepare the Chunk object wrapper
	ChunkWrapper* ChunkObject = new ChunkWrapper();
	ChunkObject->m_pChunk = Chunk;
	ChunkObject->m_pWorldObject = WorldObject;

	m_GenerateFunction(NoiseObject, ChunkObject, WorldPosition.x, WorldPosition.y, WorldPosition.z);
}
