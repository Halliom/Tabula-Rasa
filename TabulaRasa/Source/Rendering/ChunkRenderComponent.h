#pragma once

#include <list>

#include "glm\common.hpp"

#include "GL_shader.h"
#include "../Engine/Octree.h"
#include "../Game/Player.h"

struct VoxelVertex;

struct ChunkRenderData
{
	// Position Buffer Objects (PBOs)
	GLuint EastFacePBO;
	GLuint WestFacePBO;
	GLuint TopFacePBO;
	GLuint BottomFacePBO;
	GLuint NorthFacePBO;
	GLuint SouthFacePBO;

	uint32_t NumWestFaces;
	uint32_t NumTopFaces;
	uint32_t NumBottomFaces;
	uint32_t NumNorthFaces;
	uint32_t NumSouthFaces;
};

class ChunkRenderer
{
public:

	static void SetupChunkRenderer();

	static void DestroyChunkRenderer();

	static void RenderAllChunks(Player* CurrentPlayer, float CumulativeTime);

	void SetData(std::vector<VoxelVertex>& Vertices);

private:

	static GLuint EastVAO;
	static GLuint WestVAO;
	static GLuint TopVAO;
	static GLuint BottomVAO;
	static GLuint NorthVAO;
	static GLuint SouthVAO;

	static GLuint EastVBO;
	static GLuint EastIBO;

	static GLuint WestVBO;
	static GLuint WestIBO;

	static GLuint TopVBO;
	static GLuint TopIBO;

	static GLuint BottomVBO;
	static GLuint BottomIBO;

	static GLuint NorthVBO;
	static GLuint NorthIBO;

	static GLuint SouthVBO;
	static GLuint SouthIBO;

	static GLShaderProgram* ChunkRenderShader;

	static std::list<ChunkRenderData*> ChunksToRender;

};