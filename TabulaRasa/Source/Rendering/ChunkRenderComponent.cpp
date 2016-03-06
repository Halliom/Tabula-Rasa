#include "ChunkRenderComponent.h"

#include "../Engine/Octree.h"
#include "../Engine/Camera.h"
#include "../Engine/Block.h"

#include "glm\gtc\matrix_transform.hpp"

// Init the chunk list
DynamicArray<ChunkRenderData*> ChunkRenderer::ChunksToRender;

// 
GLShaderProgram* ChunkRenderer::ChunkRenderShader = NULL;

// Init the Vertex Array Object to be 0 (uninitialized)
GLuint ChunkRenderer::EastVAO;
GLuint ChunkRenderer::WestVAO;
GLuint ChunkRenderer::TopVAO;
GLuint ChunkRenderer::BottomVAO;
GLuint ChunkRenderer::NorthVAO;
GLuint ChunkRenderer::SouthVAO;

GLuint ChunkRenderer::EastVBO;
GLuint ChunkRenderer::EastIBO;

GLuint ChunkRenderer::WestVBO;
GLuint ChunkRenderer::WestIBO;

GLuint ChunkRenderer::TopVBO;
GLuint ChunkRenderer::TopIBO;

GLuint ChunkRenderer::BottomVBO;
GLuint ChunkRenderer::BottomIBO;

GLuint ChunkRenderer::NorthVBO;
GLuint ChunkRenderer::NorthIBO;

GLuint ChunkRenderer::SouthVBO;
GLuint ChunkRenderer::SouthIBO;

#define INITIAL_BUFFER_SIZE 32
#define INCREASE_AMOUNT 4

void ChunkRenderer::SetupChunkRenderer()
{
	ChunksToRender.Reserve(16);

	ChunkRenderShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("VertexShader.glsl"), std::string("FragmentShader.glsl"));

#if 0
	glGenVertexArrays(6, &EastVAO);

	// Generate all 12 buffers at once since they all should be adjacent in memory
	glGenBuffers(12, &EastVBO);

	//TODO: Change 1.0f to Voxel::CUBE_SIZE
	glm::vec3 EastFaceVertices[4] = { 
		glm::vec3(1.0f, 0.0f, 0.0f), 
		glm::vec3(1.0f, 1.0f, 0.0f), 
		glm::vec3(1.0f, 1.0f, 1.0f), 
		glm::vec3(1.0f, 0.0f, 1.0f) 
	};
	uint8_t EastFaceIndices[6] = { 3, 2, 0, 2, 1, 0 };

	glBindBuffer(GL_ARRAY_BUFFER, EastVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, EastFaceVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EastIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint8_t) * 6, EastFaceIndices, GL_STATIC_DRAW);

	glBindVertexArray(EastVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glm::vec3 WestFaceVertices[4] = { 
		glm::vec3(0.0f, 0.0f, 1.0f), 
		glm::vec3(0.0f, 1.0f, 1.0f), 
		glm::vec3(0.0f, 1.0f, 0.0f), 
		glm::vec3(0.0f, 0.0f, 0.0f) 
	};
	uint8_t WestFaceIndices[6] = { 3, 2, 0, 2, 1, 0 };

	glBindBuffer(GL_ARRAY_BUFFER, WestVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, WestFaceVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, WestIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint8_t) * 6, WestFaceIndices, GL_STATIC_DRAW);

	glBindVertexArray(WestVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glm::vec3 TopFaceVertices[4] = { 
		glm::vec3(0.0f, 1.0f, 0.0f), 
		glm::vec3(0.0f, 1.0f, 1.0f), 
		glm::vec3(1.0f, 1.0f, 1.0f), 
		glm::vec3(1.0f, 1.0f, 0.0f) 
	};
	uint8_t TopFaceIndices[6] = { 3, 2, 0, 2, 1, 0 };

	glBindBuffer(GL_ARRAY_BUFFER, TopVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, TopFaceVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TopIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint8_t) * 6, TopFaceIndices, GL_STATIC_DRAW);

	glBindVertexArray(TopVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glm::vec3 BottomFaceVertices[4] = {
		glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	};
	uint8_t BottomFaceIndices[6] = { 0, 1, 2, 0, 2, 3 };

	glBindBuffer(GL_ARRAY_BUFFER, BottomVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, BottomFaceVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BottomIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint8_t) * 6, BottomFaceIndices, GL_STATIC_DRAW);

	glBindVertexArray(BottomVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glm::vec3 NorthFaceVertices[4] = { 
		glm::vec3(0.0f, 0.0f, 1.0f), 
		glm::vec3(0.0f, 1.0f, 1.0f), 
		glm::vec3(1.0f, 1.0f, 1.0f), 
		glm::vec3(1.0f, 0.0f, 1.0f) 
	}; 
	uint8_t NorthFaceIndices[6] = { 0, 1, 2, 0, 2, 3 };

	glBindBuffer(GL_ARRAY_BUFFER, NorthVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, NorthFaceVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NorthIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint8_t) * 6, NorthFaceIndices, GL_STATIC_DRAW);

	glBindVertexArray(NorthVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glm::vec3 SouthFaceVertices[4] = { 
		glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3(0.0f, 1.0f, 0.0f), 
		glm::vec3(1.0f, 1.0f, 0.0f), 
		glm::vec3(1.0f, 0.0f, 0.0f) 
	};
	uint8_t SouthFaceIndices[6] = { 3, 2, 0, 2, 1, 0 };

	glBindBuffer(GL_ARRAY_BUFFER, SouthVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, SouthFaceVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SouthIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint8_t) * 6, SouthFaceIndices, GL_STATIC_DRAW);

	glBindVertexArray(SouthVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
#endif
}

void ChunkRenderer::DestroyChunkRenderer()
{
	glDeleteVertexArrays(6, &EastVAO);

	for (int Index = 0; Index < ChunksToRender.GetNum(); ++Index)
	{
		delete ChunksToRender[Index];
	}
	ChunksToRender.Reserve(0);

	delete ChunkRenderShader;

	glDeleteBuffers(12, &EastVBO);
}

#define PI 3.14159265359f

#include <Windows.h>

void ChunkRenderer::RenderAllChunks(Player* CurrentPlayer)
{
	if (!Camera::ActiveCamera)
	{
		return;
	}

	ChunkRenderShader->Bind();
	
	glm::mat4 Projection = *Camera::ActiveCamera->GetProjectionMatrix();
	glm::mat4 View = *Camera::ActiveCamera->GetViewMatrix();

	ChunkRenderShader->SetProjectionViewMatrix(Projection * View);

	for (int Index = 0; Index < ChunksToRender.GetNum(); ++Index)
	{
		if (!ChunksToRender[Index])
			continue;

		ChunkRenderShader->SetChunkPositionOffset(ChunksToRender[Index]->ChunkPosition);
#if 0

		// Render the east face
		glBindVertexArray(EastVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->EastFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EastIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumEastFaces);

		// Render the west face
		glBindVertexArray(WestVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->WestFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, WestIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumWestFaces);

		// Render the top face
		glBindVertexArray(TopVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->TopFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TopIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumTopFaces);

		// Render the bottom face
		glBindVertexArray(BottomVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->BottomFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BottomIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumBottomFaces);

		// Render the north face
		glBindVertexArray(NorthVAO);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->NorthFacePBO);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NorthIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumNorthFaces);

		// Render the south face
		glBindVertexArray(SouthVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->SouthFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SouthIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumSouthFaces);
#else
		glBindVertexArray(ChunksToRender[Index]->VertexArrayObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ChunksToRender[Index]->IndexBufferObject);

		glDrawElements(GL_TRIANGLES, ChunksToRender[Index]->NumVertices, GL_UNSIGNED_INT, 0);
#endif
	}
	glBindVertexArray(0);
}

int GetVoxelSide(Chunk* Chunk, const int& X, const int& Y, const int& Z, const VoxelSide& Side)
{
	Voxel* Node = Chunk->GetNodeData(glm::uvec3(X, Y, Z));
	if (Node && ((Node->SidesToRender & Side) == Side))
	{
		BlockInfo Block = BlockManager::LoadedBlocks[Node->BlockID];
		return Block.RenderType == RENDER_TYPE_SOLID ? Block.Textures[SideToInt(Side)] : -1;
	}
	return -1;
}

static void GreedyMesh(Chunk* Voxels, ChunkRenderData* RenderData)
{
	DynamicArray<glm::vec3> Vertices;
	DynamicArray<unsigned int> Indices;
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
				for (x[u] = 0; x[u] < ChunkSize; ++x[u])
				{
					for (x[v] = 0; x[v] < ChunkSize; n += 4)
					{
						int a0 = (0 <= x[d]				? GetVoxelSide(Voxels, x[0],		x[1],			x[2],			Side) : -1);
						int b0 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, x[0] + q[0], x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n] = ((a0 != -1 && b0 != -1 && a0 == b0)) ? -1 : (BackFace ? b0 : a0);
						++x[v];
						
						int a1 = (0 <= x[d]				? GetVoxelSide(Voxels, x[0],		x[1],			x[2],			Side) : -1);
						int b1 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, x[0] + q[0], x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n + 1] = ((a1 != -1 && b1 != -1 && a1 == b1)) ? -1 : (BackFace ? b1 : a1);
						++x[v];

						int a2 = (0 <= x[d]				? GetVoxelSide(Voxels, x[0],		x[1],			x[2],			Side) : -1);
						int b2 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, x[0] + q[0], x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n + 2] = ((a2 != -1 && b2 != -1 && a2 == b2)) ? -1 : (BackFace ? b2 : a2);
						++x[v];

						int a3 = (0 <= x[d]				? GetVoxelSide(Voxels, x[0],		x[1],			x[2],			Side) : -1);
						int b3 = (x[d] < ChunkSize - 1	? GetVoxelSide(Voxels, x[0] + q[0], x[1] + q[1],	x[2] + q[2],	Side) : -1);
						mask[n + 3] = ((a3 != -1 && b3 != -1 && a3 == b3)) ? -1 : (BackFace ? b3 : a3);
						++x[v];
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
							Vertices.Reserve(StartIndex + 4); // + 1 would be Vertices.size(), +4 again is the amount we're adding
							glm::vec3 QuadVert0 = glm::vec3(x[0],					x[1],					x[2]);
							glm::vec3 QuadVert1 = glm::vec3(x[0] + du[0],			x[1] + du[1],			x[2] + du[2]);
							glm::vec3 QuadVert2 = glm::vec3(x[0] + du[0] + dv[0],	x[1] + du[1] + dv[1],	x[2] + du[2] + dv[2]);
							glm::vec3 QuadVert3 = glm::vec3(x[0] + dv[0],			x[1] + dv[1],			x[2] + dv[2]);

							Vertices.Push(glm::vec3(x[0],					x[1],					x[2]));
							Vertices.Push(glm::vec3(x[0] + du[0],			x[1] + du[1],			x[2] + du[2]));
							Vertices.Push(glm::vec3(x[0] + du[0] + dv[0],	x[1] + du[1] + dv[1],	x[2] + du[2] + dv[2]));
							Vertices.Push(glm::vec3(x[0] + dv[0],			x[1] + dv[1],			x[2] + dv[2]));

							Indices.Reserve(Indices.GetNum() + 6);
							if (BackFace)
							{
								Indices.Push(StartIndex + 2);	//2
								Indices.Push(StartIndex);		//0
								Indices.Push(StartIndex + 1);	//1
								Indices.Push(StartIndex + 1);	//1
								Indices.Push(StartIndex + 3);	//3
								Indices.Push(StartIndex + 2);	//2
							}
							else
							{
								Indices.Push(StartIndex + 2);	//2
								Indices.Push(StartIndex + 3);	//3
								Indices.Push(StartIndex + 1);	//1
								Indices.Push(StartIndex + 1);	//1
								Indices.Push(StartIndex);		//0
								Indices.Push(StartIndex + 2);	//2
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * Vertices.GetNum(), &Vertices[0].x, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderData->IndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.GetNum(), &Indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindVertexArray(0); // Unbind so nothing else modifies it
}

ChunkRenderData* ChunkRenderer::CreateRenderData(const glm::vec3& Position, Chunk* Voxels)
{
	ChunkRenderData* RenderData = new ChunkRenderData();
	glGenVertexArrays(1, &RenderData->VertexArrayObject);
	glGenBuffers(2, &RenderData->VertexBufferObject);

	GreedyMesh(Voxels, RenderData);

	RenderData->ChunkPosition = Position;
	ChunksToRender.Push(RenderData);

	return RenderData;
}

__forceinline void ChunkRenderer::InsertIntoBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate& NewCoordinate)
{
#if 0
	switch (Side)
	{
		case SIDE_EAST:
		{
			InsertIntoBuffer(&RenderData->EastFacePBO, &RenderData->NumEastFaces, &RenderData->EastFacesBufferLength, NewCoordinate);
			break;
		}
		case SIDE_WEST:
		{
			InsertIntoBuffer(&RenderData->WestFacePBO, &RenderData->NumWestFaces, &RenderData->WestFacesBufferLength, NewCoordinate);
			break;
		}
		case SIDE_TOP:
		{
			InsertIntoBuffer(&RenderData->TopFacePBO, &RenderData->NumTopFaces, &RenderData->TopFacesBufferLength, NewCoordinate);
			break;
		}
		case SIDE_BOTTOM:
		{
			InsertIntoBuffer(&RenderData->BottomFacePBO, &RenderData->NumBottomFaces, &RenderData->BottomFacesBufferLength, NewCoordinate);
			break;
		}
		case SIDE_NORTH:
		{
			InsertIntoBuffer(&RenderData->NorthFacePBO, &RenderData->NumNorthFaces, &RenderData->NorthFacesBufferLength, NewCoordinate);
			break;
		}
		case SIDE_SOUTH:
		{
			InsertIntoBuffer(&RenderData->SouthFacePBO, &RenderData->NumSouthFaces, &RenderData->SouthFacesBufferLength, NewCoordinate);
			break;
		}
		default: { break; }
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
#endif
}

void ChunkRenderer::InsertIntoBuffer(GLuint* FacePBO, uint32_t* NumFaces, uint32_t* BufferLength, ChunkRenderCoordinate& NewCoordinate)
{
	glBindBuffer(GL_ARRAY_BUFFER, *FacePBO);
	uint8_t* BufferStorage = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	if ((*NumFaces) >= (*BufferLength))
	{
		// This automatically unmaps the buffer
		glBufferData(GL_ARRAY_BUFFER, ((*NumFaces) + INCREASE_AMOUNT) * 3, BufferStorage, GL_DYNAMIC_DRAW);
		*BufferLength = (*NumFaces) + INCREASE_AMOUNT;
	}
	BufferStorage += (3 * (*NumFaces));

	memcpy(BufferStorage, &NewCoordinate.X, 3);
	++(*NumFaces);
}

void ChunkRenderer::InsertBatchIntoBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate* RenderCoords, uint32_t NumRenderCoords)
{
#if 0
	switch (Side)
	{
		case SIDE_EAST:
		{
			InsertBatchIntoBuffer(&RenderData->EastFacePBO, &RenderData->NumEastFaces, &RenderData->EastFacesBufferLength, RenderCoords, NumRenderCoords);
			break;
		}
		case SIDE_WEST:
		{
			InsertBatchIntoBuffer(&RenderData->WestFacePBO, &RenderData->NumWestFaces, &RenderData->WestFacesBufferLength, RenderCoords, NumRenderCoords);
			break;
		}
		case SIDE_TOP:
		{
			InsertBatchIntoBuffer(&RenderData->TopFacePBO, &RenderData->NumTopFaces, &RenderData->TopFacesBufferLength, RenderCoords, NumRenderCoords);
			break;
		}
		case SIDE_BOTTOM:
		{
			InsertBatchIntoBuffer(&RenderData->BottomFacePBO, &RenderData->NumBottomFaces, &RenderData->BottomFacesBufferLength, RenderCoords, NumRenderCoords);
			break;
		}
		case SIDE_NORTH:
		{
			InsertBatchIntoBuffer(&RenderData->NorthFacePBO, &RenderData->NumNorthFaces, &RenderData->NorthFacesBufferLength, RenderCoords, NumRenderCoords);
			break;
		}
		case SIDE_SOUTH:
		{
			InsertBatchIntoBuffer(&RenderData->SouthFacePBO, &RenderData->NumSouthFaces, &RenderData->SouthFacesBufferLength, RenderCoords, NumRenderCoords);
			break;
		}
		default: { break; }
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
#endif
}

void ChunkRenderer::InsertBatchIntoBuffer(GLuint* FacePBO, uint32_t* NumFaces, uint32_t* BufferLength, ChunkRenderCoordinate* RenderCoords, uint32_t NumRenderCoords)
{
	glBindBuffer(GL_ARRAY_BUFFER, *FacePBO);

	uint8_t* BufferStorage = (uint8_t*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	if (((*NumFaces) + NumRenderCoords) >= (*BufferLength)) // We are going to have to reallocate
	{
		glBufferData(GL_ARRAY_BUFFER, ((*NumFaces) + NumRenderCoords + INCREASE_AMOUNT) * 3, BufferStorage, GL_DYNAMIC_DRAW);
		*BufferLength = (*NumFaces) + NumRenderCoords + INCREASE_AMOUNT;

		// Remaps the buffer since glBufferData automatically unmaps it
		BufferStorage = (uint8_t*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	}

	memcpy(BufferStorage + (*NumFaces), &RenderCoords[0].X, 3 * NumRenderCoords);
	*NumFaces += NumRenderCoords;
}

void ChunkRenderer::SpliceFromBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate& Coordinate)
{
#if 0
	switch (Side)
	{
		case SIDE_EAST:
		{
			SpliceFromBuffer(&RenderData->EastFacePBO, &RenderData->NumEastFaces, &RenderData->EastFacesBufferLength, Coordinate);
			break;
		}
		case SIDE_WEST:
		{
			SpliceFromBuffer(&RenderData->WestFacePBO, &RenderData->NumWestFaces, &RenderData->WestFacesBufferLength, Coordinate);
			break;
		}
		case SIDE_TOP:
		{
			SpliceFromBuffer(&RenderData->TopFacePBO, &RenderData->NumTopFaces, &RenderData->TopFacesBufferLength, Coordinate);
			break;
		}
		case SIDE_BOTTOM:
		{
			SpliceFromBuffer(&RenderData->BottomFacePBO, &RenderData->NumBottomFaces, &RenderData->BottomFacesBufferLength, Coordinate);
			break;
		}
		case SIDE_NORTH:
		{
			SpliceFromBuffer(&RenderData->NorthFacePBO, &RenderData->NumNorthFaces, &RenderData->NorthFacesBufferLength, Coordinate);
			break;
		}
		case SIDE_SOUTH:
		{
			SpliceFromBuffer(&RenderData->SouthFacePBO, &RenderData->NumSouthFaces, &RenderData->SouthFacesBufferLength, Coordinate);
			break;
		}
		default: { break; }
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
#endif
}

void ChunkRenderer::SpliceFromBuffer(GLuint * FacePBO, uint32_t * NumFaces, uint32_t * BufferLength, ChunkRenderCoordinate& Coordinate)
{
	glBindBuffer(GL_ARRAY_BUFFER, *FacePBO);

	if (*NumFaces == 0) // Nothing to do
		return;

	if (*NumFaces == 1)
		glBufferData(GL_ARRAY_BUFFER, INITIAL_BUFFER_SIZE * 3, NULL, GL_DYNAMIC_DRAW); // Just reset it

	uint8_t* BufferStorage = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	unsigned int Index = 0;
	do {
		if (Coordinate.X == BufferStorage[0] &&
			Coordinate.Y == BufferStorage[1] &&
			Coordinate.Z == BufferStorage[2])
		{
			// We have found the index for the position
			uint32_t UpperHalfLength = ((*BufferLength) - Index - 1) * 3; // 3 uint8_ts for every index
			if (UpperHalfLength == 0) // We found it at the last index
			{
				memset(BufferStorage, NULL, 3); // Set 3 null bytes into the end of BufferStorage
			}
			else
			{
				uint8_t* TempBuffer = new uint8_t[UpperHalfLength];
				memcpy(TempBuffer, BufferStorage + 3, UpperHalfLength);
				memcpy(BufferStorage, TempBuffer, UpperHalfLength);
				--(*NumFaces);
				delete TempBuffer;
				break;
			}
		}
		++Index;
		BufferStorage += 3;
	} while (Index < (*NumFaces));
}
