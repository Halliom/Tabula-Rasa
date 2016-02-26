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

	uint32_t NumEastFaces;
	uint32_t NumWestFaces;
	uint32_t NumTopFaces;
	uint32_t NumBottomFaces;
	uint32_t NumNorthFaces;
	uint32_t NumSouthFaces;

	uint32_t EastFacesBufferLength;
	uint32_t WestFacesBufferLength;
	uint32_t TopFacesBufferLength;
	uint32_t BottomFacesBufferLength;
	uint32_t NorthFacesBufferLength;
	uint32_t SouthFacesBufferLength;
};

class ChunkRenderer
{
public:

	static void SetupChunkRenderer();

	static void DestroyChunkRenderer();

	static void RenderAllChunks(Player* CurrentPlayer, float CumulativeTime);

	static void InsertIntoBuffer(ChunkRenderData* RenderData, const VoxelSide& Side, uint8_t NewX, uint8_t NewY, uint8_t NewZ);

	static void SpliceFromBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, uint8_t X, uint8_t Y, uint8_t Z);

	static void SpliceFromBuffer(GLuint* FacePBO, uint32_t* NumFaces, uint32_t* BufferLength, uint8_t& X, uint8_t& Y, uint8_t& Z);

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