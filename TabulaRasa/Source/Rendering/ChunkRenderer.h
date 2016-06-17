#pragma once

#include "glm\common.hpp"

#include "../Engine/Core/Memory.h"
#include "../Engine/Core/List.h"

#include "GL_shader.h"
#include "../Game/Player.h"

struct MultiblockRenderData
{
	int PositionX;
	int PositionY;
	int PositionZ;

	unsigned int BlockID;
};

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

	MultiblockRenderData *MultiblocksToRender;
	unsigned int NumMultiblocksToRender;
};

enum VoxelSide : uint32_t;

struct TexturedQuadVertex
{
	// TODO: FIX THESE (align is all kinds of fukd)
	glm::vec3 Position; /* size 12, align 4 */ 
	glm::vec3 Normal; /* size 12, align 4 */
	glm::vec2 Dimension; /* size 8, align 4 */
	unsigned char TextureCoord; /* size 4, align 4 */
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

	static void DeleteRenderData(ChunkRenderData* RenderData);

	static void UpdateRenderData(ChunkRenderData* RenderData, Chunk* Voxels);

	static List<ChunkRenderData*> g_ChunksToRender;

	static GLuint g_TextureAtlas;

private:

	static GLShaderProgram *g_ChunkRenderShader;

	static MemoryPool<ChunkRenderData>* g_RenderDataMemoryPool;

};
