#include "ChunkRenderer.h"

#include "GLFW/glfw3.h"

#include "glm/gtc/matrix_transform.hpp"

#include "../Engine/Async/ThreadSystem.h"
#include "../Engine/Async/Job.h"
#include "../Engine/Core/Memory.h"
#include "../Engine/Core/List.h"
#include "../Engine/Octree.h"
#include "../Engine/Camera.h"
#include "../Engine/Block.h"
#include "../Engine/Chunk.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Shaders/ChunkRenderShader.h"
#include "../Platform/Platform.h"
#include "../Game/Player.h"

ChunkRenderer::ChunkRenderer()
{
	m_pChunkRenderShader = new ChunkRenderShader();
}

ChunkRenderer::~ChunkRenderer()
{
    delete m_pChunkRenderShader;
}

void ChunkRenderer::SetupChunkRenderer()
{
    m_ChunksToRender = List<ChunkRenderData>(g_Engine->g_MemoryManager->m_pRenderingMemory);
    // Reserve space for 128 ChunkRenderDatas up front
    m_ChunksToRender.Reserve(128);

	// Load the texture atlas for all the blocks textures
	unsigned int Width, Height;
	std::string FileName = PlatformFileSystem::GetAssetDirectory(DT_TEXTURES).append(std::string("textures.png"));
	m_TextureAtlas = PlatformFileSystem::LoadImageFromFile((char*)FileName.c_str(), Width, Height);
    
    m_pChunkRenderShader->Initialize();
}

void ChunkRenderer::AddCustomRendererForBlock(IDType BlockID, const char* BlockModelFilename)
{
	PlatformFileSystem::LoadModel(&m_CustomBlockRenderers[BlockID], BlockModelFilename);
}

void ChunkRenderer::RenderChunks(Player* CurrentPlayer)
{
	const static glm::mat4 Identity = glm::mat4(1.0f);

	glm::mat4 Projection = Camera::g_ActiveCamera->m_ProjectionMatrix;
	glm::mat4 View = Camera::g_ActiveCamera->m_ViewMatrix;

	m_pChunkRenderShader->m_ProjectionMatrix = Projection;
	m_pChunkRenderShader->m_ViewMatrix = View;
    
	//List<ChunkRenderData> ChunksToRender = CurrentPlayer->m_pWorldObject->m_pChunkManager->GetVisibleChunks();

	for (size_t i = 0; i < m_ChunksToRender.Size; ++i)
	{
        m_pChunkRenderShader->m_ModelMatrix = glm::translate(Identity, m_ChunksToRender[i].ChunkPosition);
        
        m_pChunkRenderShader->Use();

		/*for (size_t j = 0; i < m_ChunksToRender[i].NumMultiblocksToRender; ++i)
		{
			// Fetch the model for the multiblock to render
			MultiblockRenderData* Multiblock = &m_ChunksToRender[i].MultiblocksToRender[j];
			LoadedModel Model = m_CustomBlockRenderers[Multiblock->BlockID];

			glBindVertexArray(Model.m_AssetVAO);
			glDrawElements(GL_TRIANGLES, Model.m_NumVertices, GL_UNSIGNED_SHORT, 0);
			glBindVertexArray(0);
		}*/
		
        if (m_ChunksToRender[i].NumVertices == 0)
            continue;

		m_TextureAtlas.Use();
		glBindVertexArray(m_ChunksToRender[i].VertexArrayObject);
		glDrawElements(GL_TRIANGLES, m_ChunksToRender[i].NumVertices, GL_UNSIGNED_SHORT, 0);
	}
	glBindVertexArray(0);
}

FORCEINLINE int GetVoxelSide(Chunk* Voxels, List<MultiblockRenderData>* AdditionalRenderData, const int& X, const int& Y, const int& Z, const VoxelSide& Side)
{
	Voxel* Node = Voxels->GetVoxel(X, Y, Z);
	if (Node && ((Node->SidesToRender & Side) == Side))
	{
		BlockInfo Block = BlockManager::LoadedBlocks[Node->BlockID];
		switch(Block.RenderType)
		{
			case TYPE_SOLID:
			{
				return SideHelper::GetRotatedTexture(Block, Side, Node->Rotation);
			}
			case TYPE_MULTIBLOCK:
			{
				AdditionalRenderData->Push({ X, Y, Z, Block.BlockID });
				return -1;
			}
            case TYPE_STRUCTURE:
            {
                // TODO: Add structures
                break;
            }
		}
	}
	return -1;
}

static glm::vec3 EAST_FACE_NORMAL	= glm::vec3(+1.0f, 0.0f, 0.0f);
static glm::vec3 WEST_FACE_NORMAL	= glm::vec3(-1.0f, 0.0f, 0.0f);
static glm::vec3 TOP_FACE_NORMAL	= glm::vec3(0.0f, +1.0f, 0.0f);
static glm::vec3 BOTTOM_FACE_NORMAL = glm::vec3(0.0f, -1.0f, 0.0f);
static glm::vec3 NORTH_FACE_NORMAL	= glm::vec3(0.0f, 0.0f, +1.0f);
static glm::vec3 SOUTH_FACE_NORMAL	= glm::vec3(0.0f, 0.0f, -1.0f);

static void GreedyMesh(Chunk* Voxels, ChunkRenderData* RenderData)
{
	List<TexturedQuadVertex>* Vertices = new List<TexturedQuadVertex>(g_Engine->g_MemoryManager->m_pGameMemory);
	Vertices->Reserve(32 * 32 * 32);
	List<GLushort>* Indices = new List<GLushort>(g_Engine->g_MemoryManager->m_pGameMemory);
	Indices->Reserve(32 * 32 * 32);

	List<MultiblockRenderData> AdditionalRenderData = List<MultiblockRenderData>(g_Engine->g_MemoryManager->m_pGameMemory);
	bool Counter = false;

	int ChunkSize = Chunk::SIZE;
	for (bool BackFace = true; Counter != BackFace; BackFace = BackFace && Counter, Counter = !Counter)
	{
		for (int d = 0; d < 3; ++d)
		{
			int i = 0, j = 0, k = 0, l = 0;
			int w = 0, h = 0;

			// The other two sides
			int u = (d + 1) % 3;
			int v = (d + 2) % 3;

			int x[3] = { 0, 0, 0 }; // This is just a vector of the x, y, z position
			int q[3] = { 0, 0, 0 }; // This is the offset in the direction, d, that we are currently iterating through

									// A mask of a "slice" of the cube, since we're going through
									// the chunk depth first, this contains the groups of matching
									// voxel faces in 6 directinos
			int mask[Chunk::SIZE * Chunk::SIZE];

			q[d] = 1;

			VoxelSide Side = BackFace ? SIDE_WEST : SIDE_EAST; // if d == 0
			if (d == 1)
			{
				Side = BackFace ? SIDE_BOTTOM : SIDE_TOP;
			}
			if (d == 2)
			{
				Side = BackFace ? SIDE_SOUTH : SIDE_NORTH;
			}

			for (x[d] = -1; x[d] < ChunkSize; ) // dimensions[d], but since they're all the same it just says CHUNK_WIDTH
			{
				int n = 0;
				for (x[v] = 0; x[v] < ChunkSize; ++x[v])
				{
					for (x[u] = 0; x[u] < ChunkSize; n += 4)
					{
						int a0 = (0 <= x[d]				? GetVoxelSide(Voxels, &AdditionalRenderData, x[0],			x[1],			x[2],			Side) : -1);
						int b0 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, &AdditionalRenderData, x[0] + q[0],	x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n] = (a0 != -1 && b0 != -1 && a0 == b0) ? -1 : (BackFace ? b0 : a0);
						++x[u];

						int a1 = (0 <= x[d]				? GetVoxelSide(Voxels, &AdditionalRenderData, x[0],			x[1],			x[2],			Side) : -1);
						int b1 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, &AdditionalRenderData, x[0] + q[0],	x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n + 1] = (a1 != -1 && b1 != -1 && a1 == b1) ? -1 : (BackFace ? b1 : a1);
						++x[u];

						int a2 = (0 <= x[d]				? GetVoxelSide(Voxels, &AdditionalRenderData, x[0],			x[1],			x[2],			Side) : -1);
						int b2 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, &AdditionalRenderData, x[0] + q[0],	x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n + 2] = (a2 != -1 && b2 != -1 && a2 == b2) ? -1 : (BackFace ? b2 : a2);
						++x[u];

						int a3 = (0 <= x[d]				? GetVoxelSide(Voxels, &AdditionalRenderData, x[0],			x[1],			x[2],			Side) : -1);
						int b3 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, &AdditionalRenderData, x[0] + q[0],	x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n + 3] = (a3 != -1 && b3 != -1 && a3 == b3) ? -1 : (BackFace ? b3 : a3);
						++x[u];
					}
				}

				++x[d];

				n = 0;
				for (j = 0; j < ChunkSize; ++j)
				{
					for (i = 0; i < ChunkSize; )
					{
						int CurrentBlock = mask[n];
						if (CurrentBlock != -1)
						{
							// Go until the block in the mask at that coordinate changes
							for (w = 1; (i + w) < ChunkSize && mask[n + w] != -1 && CurrentBlock == mask[n + w]; ++w)
								;

							for (h = 1; j + h < ChunkSize; ++h)
							{
								for (k = 0; k < w; ++k)
								{
									if (mask[n + k + h * ChunkSize] == -1 || CurrentBlock != mask[n + k + h * ChunkSize])
									{
										goto endloop;
									}
								}
							}
							endloop:

							x[u] = i;
							x[v] = j;
							int du[3] = { 0, 0, 0 };
							int dv[3] = { 0, 0, 0 };
							du[u] = w;
							dv[v] = h;

							GLushort StartIndex = (GLushort) Vertices->Size;

							switch (d)
							{
								case 0:
								{
									Vertices->Push({
										glm::vec3(x[0],					x[1],					x[2]),
										glm::vec3(BackFace ? WEST_FACE_NORMAL : EAST_FACE_NORMAL),
										glm::vec2((float)h, (float)w),
										(unsigned char)CurrentBlock });

									Vertices->Push({
										glm::vec3(x[0] + du[0],			x[1] + du[1],			x[2] + du[2]),
										glm::vec3(BackFace ? WEST_FACE_NORMAL : EAST_FACE_NORMAL),
										glm::vec2((float)h, 0.0f),
										(unsigned char)CurrentBlock });

									Vertices->Push({
										glm::vec3(x[0] + du[0] + dv[0],	x[1] + du[1] + dv[1],	x[2] + du[2] + dv[2]),
										glm::vec3(BackFace ? WEST_FACE_NORMAL : EAST_FACE_NORMAL),
										glm::vec2(0.0f, 0.0f),
										(unsigned char)CurrentBlock });

									Vertices->Push({
										glm::vec3(x[0] + dv[0],	x[1] + dv[1],	x[2] + dv[2]),
										glm::vec3(BackFace ? WEST_FACE_NORMAL : EAST_FACE_NORMAL),
										glm::vec2(0.0f, (float)w),
										(unsigned char) CurrentBlock });
									break;
								}
								case 1:
								{
									Vertices->Push({
										glm::vec3(x[0],					x[1],					x[2]),
										glm::vec3(BackFace ? BOTTOM_FACE_NORMAL : TOP_FACE_NORMAL),
										glm::vec2((float)h, (float)w),
										(unsigned char)CurrentBlock });

									Vertices->Push({
										glm::vec3(x[0] + du[0],			x[1] + du[1],			x[2] + du[2]),
										glm::vec3(BackFace ? BOTTOM_FACE_NORMAL : TOP_FACE_NORMAL),
										glm::vec2((float)h, 0.0f),
										(unsigned char)CurrentBlock });

									Vertices->Push({
										glm::vec3(x[0] + du[0] + dv[0],	x[1] + du[1] + dv[1],	x[2] + du[2] + dv[2]),
										glm::vec3(BackFace ? BOTTOM_FACE_NORMAL : TOP_FACE_NORMAL),
										glm::vec2(0.0f, 0.0f),
										(unsigned char)CurrentBlock });

									Vertices->Push({
										glm::vec3(x[0] + dv[0],	x[1] + dv[1],	x[2] + dv[2]),
										glm::vec3(BackFace ? BOTTOM_FACE_NORMAL : TOP_FACE_NORMAL),
										glm::vec2(0.0f, (float)w),
										(unsigned char)CurrentBlock });
									break;
								}
								case 2:
								{
									Vertices->Push({
										glm::vec3(x[0],					x[1],					x[2]),
										glm::vec3(BackFace ? SOUTH_FACE_NORMAL : NORTH_FACE_NORMAL),
										glm::vec2(0.0f, (float) h),
										(unsigned char)CurrentBlock });

									Vertices->Push({
										glm::vec3(x[0] + du[0],			x[1] + du[1],			x[2] + du[2]),
										glm::vec3(BackFace ? SOUTH_FACE_NORMAL : NORTH_FACE_NORMAL),
										glm::vec2((float)w, (float)h),
										(unsigned char)CurrentBlock });

									Vertices->Push({
										glm::vec3(x[0] + du[0] + dv[0],	x[1] + du[1] + dv[1],	x[2] + du[2] + dv[2]),
										glm::vec3(BackFace ? SOUTH_FACE_NORMAL : NORTH_FACE_NORMAL),
										glm::vec2((float)w, 0.0f),
										(unsigned char)CurrentBlock });

									Vertices->Push({
										glm::vec3(x[0] + dv[0],	x[1] + dv[1],	x[2] + dv[2]),
										glm::vec3(BackFace ? SOUTH_FACE_NORMAL : NORTH_FACE_NORMAL),
										glm::vec2(0.0f, 0.0f),
										(unsigned char)CurrentBlock });
									break;
								}
							}

							if (BackFace)
							{
								Indices->Push(StartIndex);
								Indices->Push(StartIndex + 1);
								Indices->Push(StartIndex + 2);
								Indices->Push(StartIndex + 0);
								Indices->Push(StartIndex + 2);
								Indices->Push(StartIndex + 3);
							}
							else
							{
								Indices->Push(StartIndex + 3);
								Indices->Push(StartIndex + 2);
								Indices->Push(StartIndex + 0);
								Indices->Push(StartIndex + 2);
								Indices->Push(StartIndex + 1);
								Indices->Push(StartIndex);
							}

							// Reset mask memory
							for (l = 0; l < h; ++l)
							{
								for (k = 0; k < w; ++k)
								{
									mask[n + k + l * ChunkSize] = -1;
								}
							}

							// Increment and move on
							i += w;
							n += w;
						}
						else
						{
							++i;
							++n;
						}
					}
				}
			}
		}
	}

	RenderData->NumVertices = Indices->Size;
    
    CREATE_JOB_THREEPARAM(
                          GreedyMeshRenderUpdate,
                          List<TexturedQuadVertex>*, Vertices,
                          List<GLushort>*, Indices,
                          ChunkRenderData*, RenderData,
    {
        glBindVertexArray(RenderData->VertexArrayObject);
        
        glBindBuffer(GL_ARRAY_BUFFER, RenderData->VertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedQuadVertex) * Vertices->Size, Vertices->Data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderData->IndexBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * Indices->Size, Indices->Data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadVertex), (void*) offsetof(TexturedQuadVertex, Position));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadVertex), (void*) offsetof(TexturedQuadVertex, Normal));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadVertex), (void*) offsetof(TexturedQuadVertex, Dimension));
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(TexturedQuadVertex),  (void*) offsetof(TexturedQuadVertex, TextureCoord));
        
        glBindVertexArray(0);
        
        delete Vertices;
        delete Indices;
    });
    
    // Send it to the render thread
    g_Engine->g_WorldRenderer->ScheduleRenderJob(GreedyMeshRenderUpdate);
    
	if (AdditionalRenderData.Size > 0)
	{
		RenderData->MultiblocksToRender = new MultiblockRenderData[AdditionalRenderData.Size];
		memcpy(RenderData->MultiblocksToRender, &AdditionalRenderData[0], sizeof(MultiblockRenderData) * AdditionalRenderData.Size);
		RenderData->NumMultiblocksToRender = AdditionalRenderData.Size;
	}
	else
	{
		RenderData->MultiblocksToRender = NULL;
		RenderData->NumMultiblocksToRender = 0;
	}
}

ChunkRenderData ChunkRenderer::CreateRenderData(const glm::vec3& Position)
{
	// Allocate a new RenderData object and zero it out
    ChunkRenderData& RenderData = m_ChunksToRender.PushEmpty();
	memset(&RenderData, NULL, sizeof(ChunkRenderData));

	// Generate all of the OpenGL buffers
	glGenVertexArrays(1, &RenderData.VertexArrayObject);
	glGenBuffers(1, &RenderData.VertexBufferObject);
	glGenBuffers(1, &RenderData.IndexBufferObject);

	// Set the position
	RenderData.ChunkPosition = Position;
    
	return RenderData;
}

void ChunkRenderer::DeleteRenderData(const glm::vec3& ChunkPosition)
{
    ChunkRenderData* RenderData = GetRenderData(ChunkPosition);
    glDeleteVertexArrays(1, &RenderData->VertexArrayObject);
    glDeleteBuffers(1, &RenderData->VertexBufferObject);
    glDeleteBuffers(1, &RenderData->IndexBufferObject);
    
    if (RenderData->MultiblocksToRender != NULL)
    {
        delete[] RenderData->MultiblocksToRender;
        RenderData->NumMultiblocksToRender = NULL;
    }
    
    RenderData->NumVertices = 0;
    RenderData->NumMultiblocksToRender = 0;
    
	// Remove it from the list and deallocate it from the memory pool
	m_ChunksToRender.Remove(*RenderData);
}

void ChunkRenderer::UpdateRenderData(const glm::vec3& ChunkPosition, Chunk* Voxels)
{
    ChunkRenderData* Item = GetRenderData(ChunkPosition);
    
    CREATE_JOB_TWOPARAM(
                        GreedyMeshJob,
                        Chunk*, Voxels,
                        ChunkRenderData*, Item,
                        GreedyMesh(Voxels, Item););
    
	ThreadSystem::ScheduleJob(GreedyMeshJob);
}

ChunkRenderData* ChunkRenderer::GetRenderData(const glm::vec3& ChunkPosition)
{
    for (size_t i = 0; i < m_ChunksToRender.Size; ++i)
    {
        if (m_ChunksToRender[i].ChunkPosition == ChunkPosition)
            return &m_ChunksToRender[i];
    }
    return NULL;
}
