#include "ChunkRenderer.h"

#include "../Engine/Octree.h"
#include "../Engine/Camera.h"
#include "../Engine/Block.h"
#include "../Engine/Chunk.h"
#include "../Platform/Platform.h"

#include "glm\gtc\matrix_transform.hpp"

// Init the chunk list
DynamicArray<ChunkRenderData*> ChunkRenderer::g_ChunksToRender;

GLShaderProgram* ChunkRenderer::g_ChunkRenderShader = NULL;

GLuint ChunkRenderer::g_TextureAtlas;

void ChunkRenderer::SetupChunkRenderer()
{
	g_ChunksToRender.Reserve(16);

	g_ChunkRenderShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("VertexShader.glsl"), std::string("FragmentShader.glsl"));

	glGenTextures(1, &g_TextureAtlas);
	glBindTexture(GL_TEXTURE_2D, g_TextureAtlas);

	unsigned int Width, Height;
	std::string FileName = PlatformFileSystem::GetAssetDirectory(DT_TEXTURES)->append(std::string("textures.png"));
	std::vector<unsigned char>*	FontImage = PlatformFileSystem::LoadImageFromFile(FileName, Width, Height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(FontImage->at(0)));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void ChunkRenderer::DestroyChunkRenderer()
{
	for (int Index = 0; Index < g_ChunksToRender.GetNum(); ++Index)
	{
		delete g_ChunksToRender[Index];
	}
	g_ChunksToRender.Reserve(0);

	delete g_ChunkRenderShader;
}

#define PI 3.14159265359f

#include <Windows.h>

void ChunkRenderer::RenderAllChunks(Player* CurrentPlayer)
{
	g_ChunkRenderShader->Bind();

	glm::mat4 Projection = *Camera::ActiveCamera->GetProjectionMatrix();
	glm::mat4 View = *Camera::ActiveCamera->GetViewMatrix();

	glBindTexture(GL_TEXTURE_2D, g_TextureAtlas);
	g_ChunkRenderShader->SetProjectionMatrix(Projection);
	g_ChunkRenderShader->SetViewMatrix(View);

	const glm::mat4 Identity = glm::mat4(1.0f);

	for (int Index = 0; Index < g_ChunksToRender.GetNum(); ++Index)
	{
		if (!g_ChunksToRender[Index])
			continue;

		g_ChunkRenderShader->SetModelMatrix(glm::translate(Identity, g_ChunksToRender[Index]->ChunkPosition));

		glBindVertexArray(g_ChunksToRender[Index]->VertexArrayObject);
		glDrawElements(GL_TRIANGLES, g_ChunksToRender[Index]->NumVertices, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
}

int GetVoxelSide(Chunk* Voxels, const int& X, const int& Y, const int& Z, const VoxelSide& Side)
{
	Voxel* Node = Voxels->GetVoxel(X, Y, Z);
	if (Node && ((Node->SidesToRender & Side) == Side))
	{
		BlockInfo Block = BlockManager::LoadedBlocks[Node->BlockID];
		return Block.RenderType == TYPE_SOLID ? Block.Textures[SideToInt(Side)] : -1;
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
	DynamicArray<TexturedQuadVertex> Vertices;
	DynamicArray<unsigned int> Indices;
	bool Counter = false;

	int ChunkSize = Octree<Voxel>::SIZE;
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
			int mask[Octree<Voxel>::SIZE * Octree<Voxel>::SIZE];

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
						int a0 = (0 <= x[d]				? GetVoxelSide(Voxels, x[0],		x[1],			x[2],			Side) : -1);
						int b0 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, x[0] + q[0], x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n] = (a0 != -1 && b0 != -1 && a0 == b0) ? -1 : (BackFace ? b0 : a0);
						++x[u];

						int a1 = (0 <= x[d]				? GetVoxelSide(Voxels, x[0],		x[1],			x[2],			Side) : -1);
						int b1 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, x[0] + q[0], x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n + 1] = (a1 != -1 && b1 != -1 && a1 == b1) ? -1 : (BackFace ? b1 : a1);
						++x[u];

						int a2 = (0 <= x[d]				? GetVoxelSide(Voxels, x[0],		x[1],			x[2],			Side) : -1);
						int b2 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, x[0] + q[0], x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n + 2] = (a2 != -1 && b2 != -1 && a2 == b2) ? -1 : (BackFace ? b2 : a2);
						++x[u];

						int a3 = (0 <= x[d]				? GetVoxelSide(Voxels, x[0],		x[1],			x[2],			Side) : -1);
						int b3 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, x[0] + q[0], x[1] + q[1],	x[2] + q[2],	Side) : -1);
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

							// Reserve 4 vertices
							unsigned int StartIndex = Vertices.GetNum();
							Vertices.Reserve(StartIndex + 4); // +4 again is the amount we're adding

							switch (d)
							{
								case 0:
								{
									Vertices.Push({ glm::vec3(x[0],					x[1],					x[2]),
										glm::vec3(BackFace ? WEST_FACE_NORMAL : EAST_FACE_NORMAL),
										glm::vec2((float)h, (float)w),
										(unsigned char)CurrentBlock });

									Vertices.Push({ glm::vec3(x[0] + du[0],			x[1] + du[1],			x[2] + du[2]),
										glm::vec3(BackFace ? WEST_FACE_NORMAL : EAST_FACE_NORMAL),
										glm::vec2((float)h, 0.0f),
										(unsigned char)CurrentBlock });

									Vertices.Push({ glm::vec3(x[0] + du[0] + dv[0],	x[1] + du[1] + dv[1],	x[2] + du[2] + dv[2]),
										glm::vec3(BackFace ? WEST_FACE_NORMAL : EAST_FACE_NORMAL),
										glm::vec2(0.0f, 0.0f),
										(unsigned char)CurrentBlock });

									Vertices.Push({ glm::vec3(x[0] + dv[0],	x[1] + dv[1],	x[2] + dv[2]),
										glm::vec3(BackFace ? WEST_FACE_NORMAL : EAST_FACE_NORMAL),
										glm::vec2(0.0f, (float)w),
										(unsigned char)CurrentBlock });
									break;
								}
								case 1:
								{
									Vertices.Push({ glm::vec3(x[0],					x[1],					x[2]),
										glm::vec3(BackFace ? BOTTOM_FACE_NORMAL : TOP_FACE_NORMAL),
										glm::vec2((float)h, (float)w),
										(unsigned char)CurrentBlock });

									Vertices.Push({ glm::vec3(x[0] + du[0],			x[1] + du[1],			x[2] + du[2]),
										glm::vec3(BackFace ? BOTTOM_FACE_NORMAL : TOP_FACE_NORMAL),
										glm::vec2((float)h, 0.0f),
										(unsigned char)CurrentBlock });

									Vertices.Push({ glm::vec3(x[0] + du[0] + dv[0],	x[1] + du[1] + dv[1],	x[2] + du[2] + dv[2]),
										glm::vec3(BackFace ? BOTTOM_FACE_NORMAL : TOP_FACE_NORMAL),
										glm::vec2(0.0f, 0.0f),
										(unsigned char)CurrentBlock });

									Vertices.Push({ glm::vec3(x[0] + dv[0],	x[1] + dv[1],	x[2] + dv[2]),
										glm::vec3(BackFace ? BOTTOM_FACE_NORMAL : TOP_FACE_NORMAL),
										glm::vec2(0.0f, (float)w),
										(unsigned char)CurrentBlock });
									break;
								}
								case 2:
								{
									Vertices.Push({ glm::vec3(x[0],					x[1],					x[2]),
										glm::vec3(BackFace ? SOUTH_FACE_NORMAL : NORTH_FACE_NORMAL),
										glm::vec2(0.0f, (float) h),
										(unsigned char)CurrentBlock });

									Vertices.Push({ glm::vec3(x[0] + du[0],			x[1] + du[1],			x[2] + du[2]),
										glm::vec3(BackFace ? SOUTH_FACE_NORMAL : NORTH_FACE_NORMAL),
										glm::vec2((float)w, (float)h),
										(unsigned char)CurrentBlock });

									Vertices.Push({ glm::vec3(x[0] + du[0] + dv[0],	x[1] + du[1] + dv[1],	x[2] + du[2] + dv[2]),
										glm::vec3(BackFace ? SOUTH_FACE_NORMAL : NORTH_FACE_NORMAL),
										glm::vec2((float)w, 0.0f),
										(unsigned char)CurrentBlock });

									Vertices.Push({ glm::vec3(x[0] + dv[0],	x[1] + dv[1],	x[2] + dv[2]),
										glm::vec3(BackFace ? SOUTH_FACE_NORMAL : NORTH_FACE_NORMAL),
										glm::vec2(0.0f, 0.0f),
										(unsigned char)CurrentBlock });
									break;
								}
							}

							Indices.Reserve(Indices.GetNum() + 6);
							if (BackFace)
							{
								Indices.Push(StartIndex);
								Indices.Push(StartIndex + 1);
								Indices.Push(StartIndex + 2);
								Indices.Push(StartIndex + 0);
								Indices.Push(StartIndex + 2);
								Indices.Push(StartIndex + 3);
							}
							else
							{
								Indices.Push(StartIndex + 3);
								Indices.Push(StartIndex + 2);
								Indices.Push(StartIndex + 0);
								Indices.Push(StartIndex + 2);
								Indices.Push(StartIndex + 1);
								Indices.Push(StartIndex);
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

	RenderData->NumVertices = Indices.GetNum();

	glBindVertexArray(RenderData->VertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, RenderData->VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedQuadVertex) * Vertices.GetNum(), &Vertices[0].Position.x, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderData->IndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.GetNum(), &Indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadVertex), (void*) offsetof(TexturedQuadVertex, Position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadVertex), (void*) offsetof(TexturedQuadVertex, Normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedQuadVertex), (void*) offsetof(TexturedQuadVertex, Dimension));
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(TexturedQuadVertex),  (void*) offsetof(TexturedQuadVertex, TextureCoord));

	// Unbind so nothing else modifies it
	glBindVertexArray(0);
}

ChunkRenderData* ChunkRenderer::CreateRenderData(const glm::vec3& Position, Chunk* Voxels)
{
	ChunkRenderData* RenderData = new ChunkRenderData();
	glGenVertexArrays(1, &RenderData->VertexArrayObject);
	glGenBuffers(2, &RenderData->VertexBufferObject);

	RenderData->ChunkPosition = Position;
	g_ChunksToRender.Push(RenderData);

	return RenderData;
}

void ChunkRenderer::UpdateRenderData(ChunkRenderData* RenderData, Chunk* Voxels)
{
	GreedyMesh(Voxels, RenderData);
}
