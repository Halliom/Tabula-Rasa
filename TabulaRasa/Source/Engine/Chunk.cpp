#include "Chunk.h"

#include "../Rendering/ChunkRenderer.h"
#include "Octree.h"

Chunk::Chunk()
{
	m_bIsRenderStateDirty = false;

	// Make sure that everything is initialized to zero
	memset(m_pVoxels, NULL, 32 * 32 * 32 * sizeof(Voxel));
}

Chunk::~Chunk()
{

}

void Chunk::Initialize()
{
	// Create render data for the chunk
	m_pChunkRenderData = ChunkRenderer::CreateRenderData(
		glm::vec3(m_ChunkX * Octree<Voxel>::SIZE, m_ChunkY * Octree<Voxel>::SIZE, m_ChunkZ * Octree<Voxel>::SIZE),
		this);
}

void Chunk::Tick(float DeltaTime)
{
	if (m_bIsRenderStateDirty)
	{
		ChunkRenderData* RenderData = m_pChunkRenderData;
		ChunkRenderer::UpdateRenderData(RenderData, this);
		m_bIsRenderStateDirty = false;
	}
}
