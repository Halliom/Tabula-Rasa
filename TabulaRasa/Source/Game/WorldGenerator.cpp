#include "WorldGenerator.h"

#include "tinydir.h"

#include "../Platform/Platform.h"
#include "../Engine/Noise.h"
#include "../Game/World.h"
#include "../Engine/Chunk.h"
#include "../Engine/ScriptEngine.h"
#include "../Engine/Console.h"

float WorldGenerator::g_ScaleFactor = 50.0f;
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
#if 0
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
#else
    std::string DirectoryCppString = PlatformFileSystem::GetAssetDirectory(DT_SCRIPTS).c_str();
#ifdef _WIN32
    DirectoryCppString.append("WorldGen\\");
#else
    DirectoryCppString.append("WorldGen/");
#endif
    const char* Directory = DirectoryCppString.c_str();
    char Path[2048];
    
    tinydir_dir dir;
    tinydir_open(&dir, Directory);
    
    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);
        
        if (!file.is_dir)
        {
#ifdef _WIN32
            sprintf(Path, "WorldGen\\%s", file.name);
#else
            sprintf(Path, "WorldGen/%s", file.name);
#endif
            
            // Load the script
            g_Engine->g_ScriptEngine->ExecuteScript(Path);
        }
        tinydir_next(&dir);
    }
    
    tinydir_close(&dir);  /* could not open directory */
#endif

	// At this point "world_gen.lua" should have been loaded and so we
	// should have access to the table "Biomes" and each one of those
	// biomes should also have been loaded by now

	// Get a reference to the "Biomes" table and push some values onto the stack to
	// be able to iterate through the table
	LuaTable* BiomesTable = g_Engine->g_ScriptEngine->GetTable("Biomes");
	for (TableIterator It = BiomesTable->GetIterator(); It.HasNext; ++It)
	{
        std::string BiomeName = std::string(It.Value().String);

		// Add the biome
		BiomeScript* Biome = new BiomeScript(BiomeName);
		AddBiome(Biome);
	}
}

void WorldGenerator::GenerateChunk(glm::ivec3 ChunkPosition, Chunk* Result)
{
	int Heat = (int)(((m_HeatmapGenerator->Noise(ChunkPosition.x / g_ScaleFactor, ChunkPosition.z / g_ScaleFactor) + 1.0f) * 0.5f) * g_MaxHeatTiers);	
	int Height = ChunkPosition.y * Chunk::SIZE;

	IBiome* ChosenBiome = NULL;
	for (int i = 0; i < m_NumBiomes; ++i)
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
	for (unsigned int i = 0; i < ArrayCount(m_Features); ++i)
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
        LuaTable* FeatureTable = g_Engine->g_ScriptEngine->GetTable(FeatureName.c_str());
        Feature = new ScriptedFeature(FeatureName);
        WorldGenerator::LoadFeature(FeatureName, Feature);
	}

	// Insert at the specified order
	m_Features[Order] = Feature;
}

void IBiome::Generate(Chunk* Chunk, World* WorldObject, SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition)
{
	for (size_t i = 0; i < ArrayCount(m_Features); ++i)
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
	m_BiomeName(BiomeName)
{
	m_BiomeInfoTable = g_Engine->g_ScriptEngine->GetTable(m_BiomeName.c_str());

	// Set the values for the biome picking part of the WorldGenerator
	m_MinHeatLevel = (int)m_BiomeInfoTable->GetValue("min_heat").Number;
	m_MaxHeatLevel = (int)m_BiomeInfoTable->GetValue("max_heat").Number;
	m_MinHeight = (int)m_BiomeInfoTable->GetValue("min_height").Number;
	m_MaxHeight = (int)m_BiomeInfoTable->GetValue("max_height").Number;

	LuaTable* Features = m_BiomeInfoTable->GetValue("features").Table;

	// Iterate through all of the features listed
	for (TableIterator It = Features->GetIterator(); It.HasNext; ++It)
	{
		// Add the feature generator (by its name), note that It.key()
		// is not zero-based, therefore we subtract one off it
        std::string FeatureName = std::string(It.Value().String);
		int Key = (int)It.Key().Number;
		AddFeatureGenerator(Key - 1, FeatureName);
		
		LogF("Biome \"%s\" loaded feature generator \"%s\"", m_BiomeName.c_str(), FeatureName.c_str());
	}
}

BiomeScript::~BiomeScript()
{
}

ScriptedFeature::ScriptedFeature(std::string Name) : 
	IFeature(Name)
{
	m_FeatureTable = g_Engine->g_ScriptEngine->GetTable(Name.c_str());

	m_GenerateFunction = m_FeatureTable->GetValue("generate");
}

void ScriptedFeature::GenerateToChunk(Chunk* Chunk, World* WorldObject, SimplexNoise* NoiseGenerator, glm::ivec3 WorldPosition)
{
	// Prepare the Noise object wrapper
	SimplexNoiseWrapper* NoiseObject = new SimplexNoiseWrapper();
	NoiseObject->m_pNoise = NoiseGenerator;

	// Prepare the Chunk object wrapper
	ChunkWrapper* ChunkObject = new ChunkWrapper();
	ChunkObject->m_pChunk = Chunk;
	ChunkObject->m_pWorldObject = WorldObject;

    // TODO: Fix this with userdata support
    /* Scripts::Call(
                  m_GenerateFunction.Function,
                  NoiseObject,
                  ChunkObject,
                  WorldPosition.x,
                  WorldPosition.y,
                  WorldPosition.z); */
    
    g_Engine->g_ScriptEngine->SetupFunction(m_GenerateFunction.Function);
    luabridge::Stack<SimplexNoiseWrapper*>::push(g_Engine->g_ScriptEngine->L, NoiseObject);
    luabridge::Stack<ChunkWrapper*>::push(g_Engine->g_ScriptEngine->L, ChunkObject);
    g_Engine->g_ScriptEngine->PushValue(WorldPosition.x);
    g_Engine->g_ScriptEngine->PushValue(WorldPosition.y);
    g_Engine->g_ScriptEngine->PushValue(WorldPosition.z);
    
    g_Engine->g_ScriptEngine->CallFunction(5);
}
