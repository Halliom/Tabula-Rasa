#pragma once

#include "glm\common.hpp"
#include "DynamicArray.cpp"

#include "GL_shader.h"
#include "../Game/Player.h"

struct ChunkRenderData
{
	// The position (in world coordinates) that the chunk is in
	glm::vec3 ChunkPosition;

	// Holds all the state for the rendering
	GLuint VertexArrayObject;

	// Vertex buffer and index buffer for the chunk
	GLuint VertexBufferObject;
	GLuint IndexBufferObject;

	unsigned int NumVertices;
};

enum VoxelSide : uint32_t;

struct TexturedQuadVertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 Dimension;
	unsigned char TextureCoord;
};

template<typename T> class Octree;

class Voxel;
class Chunk;

class ChunkRenderer
{
public:

	static void SetupChunkRenderer();

	static void DestroyChunkRenderer();

	static void RenderAllChunks(Player* CurrentPlayer);

	static ChunkRenderData* CreateRenderData(const glm::vec3& Position, Chunk* Voxels);

	static void UpdateRenderData(ChunkRenderData* RenderData, Chunk* Voxels);

	static DynamicArray<ChunkRenderData*> g_ChunksToRender;

	static GLuint g_TextureAtlas;

private:

	static GLShaderProgram* g_ChunkRenderShader;

};
