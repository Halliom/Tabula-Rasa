#pragma once

#include "glm\common.hpp"
#include "DynamicArray.cpp"

#include "GL_shader.h"
#include "../Game/Player.h"

struct VoxelVertex;

struct ChunkRenderData
{
	// The position (in world coordinates) that the chunk is in
	glm::vec3 ChunkPosition;

	GLuint VertexBufferObject;
	GLuint IndexBufferObject;
};

struct ChunkRenderCoordinate
{
	ChunkRenderCoordinate(uint8_t X, uint8_t Y, uint8_t Z) : X(X), Y(Y), Z(Z) {}
	ChunkRenderCoordinate() : X(0), Y(0), Z(0) {}
	__forceinline bool operator==(const ChunkRenderCoordinate& Other)
	{
		return X == Other.X && Y == Other.Y && Z == Other.Z;
	}
	uint8_t X;
	uint8_t Y;
	uint8_t Z;
};

enum VoxelSide : uint32_t;

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

	static ChunkRenderData* CreateRenderData(const glm::vec3& Position, class Chunk* Voxels);

	static void InsertIntoBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate& NewCoordinate);

	static void InsertIntoBuffer(GLuint* FacePBO, uint32_t* NumFaces, uint32_t* BufferLength, ChunkRenderCoordinate& NewCoordinate);

	static void InsertBatchIntoBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate* RenderCoords, uint32_t NumRenderCoords);

	static void InsertBatchIntoBuffer(GLuint* FacePBO, uint32_t* NumFaces, uint32_t* BufferLength, ChunkRenderCoordinate* RenderCoords, uint32_t NumRenderCoords);

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
