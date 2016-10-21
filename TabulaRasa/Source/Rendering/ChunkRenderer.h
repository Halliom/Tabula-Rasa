#pragma once
#include <unordered_map>

#include "GL/glew.h"
#include "glm/common.hpp"

#include "../Platform/Platform.h"

#include "../Engine/Block.h"
#include "../Engine/Core/Memory.h"
#include "../Engine/Core/List.h"

#include "../Rendering/Texture.h"

template<typename T> class Octree;
template<typename T> class List;
class Voxel;
class Chunk;

struct MultiblockRenderData
{
	int PositionX;
	int PositionY;
	int PositionZ;

	IDType BlockID;
};

// Make this a separate class which is owned by Chunk and
// can update itself on invocation from its owning chunk.
// Then in GetVisibleChunks, just create a list of ChunkRenderDatas
// and pass them to the ChunkRenderer
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

	// TODO: Sort these after increasing BlockID for better cache utilization?
	MultiblockRenderData*	MultiblocksToRender;
	unsigned int			NumMultiblocksToRender;
    
    bool operator==(const ChunkRenderData& Other)
    {
        return ChunkPosition == Other.ChunkPosition &&
               VertexArrayObject == Other.VertexArrayObject;
    }
};

enum VoxelSide : uint32_t;

typedef std::unordered_map<IDType, std::vector<glm::ivec3>> RendererMap;

struct TexturedQuadVertex
{
	// TODO: FIX THESE (align is all kinds of fukd)
	glm::vec3 Position; /* size 12, align 4 */ 
	glm::vec3 Normal; /* size 12, align 4 */
	glm::vec2 Dimension; /* size 8, align 4 */
	unsigned char TextureCoord; /* size 4, align 4 */
};

class ChunkRenderer
{
public:
    
    ChunkRenderer();
    
    ~ChunkRenderer();

	void SetupChunkRenderer();

	void AddCustomRendererForBlock(IDType BlockID, const char* BlockModelFilename);

	void RenderChunks(class Player* CurrentPlayer);

	ChunkRenderData	CreateRenderData(const glm::vec3& Position);
    void			DeleteRenderData(const glm::vec3& ChunkPosition);
	void			UpdateRenderData(const glm::vec3& ChunkPosition, Chunk* Voxels);
    
private:
    
    ChunkRenderData* GetRenderData(const glm::vec3& ChunkPosition);
    
    class ChunkRenderShader*	m_pChunkRenderShader;
	Texture						m_TextureAtlas;
    List<ChunkRenderData>       m_ChunksToRender;
	LoadedModel					m_CustomBlockRenderers[MAX_NUM_BLOCKS];
};
