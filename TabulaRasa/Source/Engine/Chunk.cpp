#include "Chunk.h"

#include "../Engine/Engine.h"
#include "../Rendering/RenderingEngine.h"
#include "../Rendering/ChunkRenderer.h"
#include "Octree.h"

constexpr int Chunk::SIZE;

Chunk::Chunk() :
    m_bIsRenderStateDirty(false),
    m_pChunkRenderer(NULL)
{
	// Make sure that everything is initialized to zero
	memset(m_pVoxels, NULL, 32 * 32 * 32 * sizeof(Voxel));
}

Chunk::~Chunk()
{
    m_pChunkRenderer->DeleteRenderData(glm::vec3(m_ChunkX * SIZE, m_ChunkY * SIZE, m_ChunkZ * SIZE));
}

void Chunk::Initialize(ChunkRenderer* Renderer)
{
	// Create render data for the chunk
    m_pChunkRenderer = Renderer;
    m_pChunkRenderer->CreateRenderData(glm::vec3(m_ChunkX * SIZE, m_ChunkY * SIZE, m_ChunkZ * SIZE));
}

void Chunk::Tick(float DeltaTime)
{
	if (m_bIsRenderStateDirty)
	{
		m_pChunkRenderer->UpdateRenderData(glm::vec3(m_ChunkX * SIZE, m_ChunkY * SIZE, m_ChunkZ * SIZE), this);
		m_bIsRenderStateDirty = false;
	}
}
