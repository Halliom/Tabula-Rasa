#pragma once

#include "glm\common.hpp"
#include "DynamicArray.cpp"

#include "GL_shader.h"
#include "../Engine/Octree.h"
#include "../Game/Player.h"

struct VoxelVertex;

struct ChunkRenderData
{
	// The position (in world coordinates) that the chunk is in
	glm::vec3 ChunkPosition;

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

struct ChunkRenderCoordinate
{
	ChunkRenderCoordinate(uint8_t X, uint8_t Y, uint8_t Z) : X(X), Y(Y), Z(Z) {}
	uint8_t X;
	uint8_t Y;
	uint8_t Z;
};

struct SideCoordinate
{
	VoxelSide Side;
	uint8_t X;
	uint8_t Y;
	uint8_t Z;
};

class ChunkRenderer
{
public:

	static void SetupChunkRenderer();

	static void DestroyChunkRenderer();

	static void RenderAllChunks(Player* CurrentPlayer, float CumulativeTime);

	static void InsertIntoBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate& NewCoordinate);

	static void InsertIntoBuffer(GLuint* FacePBO, uint32_t* NumFaces, uint32_t* BufferLength, ChunkRenderCoordinate& NewCoordinate);

	static void InsertBatchIntoBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate* RenderCoords, uint32_t& NumRenderCoords);

	static void InsertBatchIntoBuffer(GLuint* FacePBO, uint32_t* NumFaces, uint32_t* BufferLength, ChunkRenderCoordinate* RenderCoords, uint32_t& NumRenderCoords);

	static void SpliceFromBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate& Coordinate);

	static void SpliceFromBuffer(GLuint* FacePBO, uint32_t* NumFaces, uint32_t* BufferLength, ChunkRenderCoordinate& Coordinate);

	void SetData(std::vector<VoxelVertex>& Vertices);

	static DynamicArray<ChunkRenderData*> ChunksToRender;

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

};