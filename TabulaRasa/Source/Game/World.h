#pragma once

#include "../Rendering/GL_shader.h"

#include "../Rendering/TextRender.h"

#include "../Engine/Octree.h"
#include "Player.h"


class World
{
public:
	World();

	~World();

	void Initialize();

	void Update(float DeltaTime);

	void AddBlock(const int& X, const int& Y, const int& Z, const unsigned int& BlockID);

	void RemoveBlock(const int& X, const int& Y, const int& Z);

	Chunk* LoadChunk(const int& ChunkX, const int& ChunkY, const int& ChunkZ);

	Player* CurrentPlayer;

private:

	Chunk** LoadedChunks;

	int ChunkLoadingCenterX;
	int ChunkLoadingCenterY;
	int ChunkLoadingCenterZ;

	unsigned int NumLoadedChunks;

	GLShaderProgram* Shader;
};