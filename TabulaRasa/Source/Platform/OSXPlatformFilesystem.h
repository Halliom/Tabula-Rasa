#pragma once

#include <string>
#include <vector>

#include "GL/glew.h"

#include "../Engine/Engine.h"
#include "../Rendering/Texture.h"

enum AssetDirectoryType
{
    DT_SHADERS,
    DT_FONTS,
    DT_TEXTURES,
    DT_MODELS,
    DT_SCRIPTS
};

struct LoadedModel
{
    /**
     * Vertex data
     */
    GLuint m_AssetVAO;
    GLuint m_AssetVBO;
    GLuint m_AssetIBO;
    
    /**
     * Data specific to rendering of the model
     */
    unsigned int m_NumVertices;
};

class PlatformFileSystem
{
public:
    static std::string LoadFile(const AssetDirectoryType& Directory, const char* FileName);
    
    //static void WriteToFile(const AssetDirectoryType& Directory, const char* FileName, const std::string& Content);
    
    static FORCEINLINE std::string LoadFile(const AssetDirectoryType& Directory, const std::string& FileName)
    {
        return LoadFile(Directory, FileName.c_str());
    }
    
    static std::string GetAssetDirectory(const AssetDirectoryType& Directory);
    
    static Texture LoadImageFromFile(const std::string& FileName, unsigned int& OutWidth, unsigned int& OutHeight);
    
    static GLuint LoadBitmapFromFile(char* FileName);
    
    static void LoadModel(LoadedModel* Model, const char* FileName);
    
    static char* LoadScript(const char* ScriptName);
};
