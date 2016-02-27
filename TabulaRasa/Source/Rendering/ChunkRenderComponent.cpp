#include "ChunkRenderComponent.h"

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

	glGenVertexArrays(6, &EastVAO);

	ChunkRenderShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("VertexShader.glsl"), std::string("FragmentShader.glsl"));

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

	//------------------------------------------
	
	ChunkRenderData* RenderData = new ChunkRenderData();
	glGenBuffers(6, &RenderData->EastFacePBO);

	glBindBuffer(GL_ARRAY_BUFFER, RenderData->EastFacePBO);
	glBufferData(GL_ARRAY_BUFFER, INITIAL_BUFFER_SIZE * 3, NULL, GL_DYNAMIC_DRAW);
	RenderData->EastFacesBufferLength = INITIAL_BUFFER_SIZE;

	glBindBuffer(GL_ARRAY_BUFFER, RenderData->WestFacePBO);
	glBufferData(GL_ARRAY_BUFFER, INITIAL_BUFFER_SIZE * 3, NULL, GL_DYNAMIC_DRAW);
	RenderData->WestFacesBufferLength = INITIAL_BUFFER_SIZE;

	glBindBuffer(GL_ARRAY_BUFFER, RenderData->TopFacePBO);
	glBufferData(GL_ARRAY_BUFFER, INITIAL_BUFFER_SIZE * 3, NULL, GL_DYNAMIC_DRAW);
	RenderData->TopFacesBufferLength = INITIAL_BUFFER_SIZE;

	glBindBuffer(GL_ARRAY_BUFFER, RenderData->BottomFacePBO);
	glBufferData(GL_ARRAY_BUFFER, INITIAL_BUFFER_SIZE * 3, NULL, GL_DYNAMIC_DRAW);
	RenderData->BottomFacesBufferLength = INITIAL_BUFFER_SIZE;

	glBindBuffer(GL_ARRAY_BUFFER, RenderData->NorthFacePBO);
	glBufferData(GL_ARRAY_BUFFER, INITIAL_BUFFER_SIZE * 3, NULL, GL_DYNAMIC_DRAW);
	RenderData->NorthFacesBufferLength = INITIAL_BUFFER_SIZE;

	glBindBuffer(GL_ARRAY_BUFFER, RenderData->SouthFacePBO);
	glBufferData(GL_ARRAY_BUFFER, INITIAL_BUFFER_SIZE * 3, NULL, GL_DYNAMIC_DRAW);
	RenderData->SouthFacesBufferLength = INITIAL_BUFFER_SIZE;

	RenderData->ChunkPosition = glm::vec3(-1.0f, -1.0f, -5.0f);

	InsertIntoBufferSide(RenderData, VoxelSide::SIDE_NORTH, ChunkRenderCoordinate(0, 0, 0));
	InsertIntoBufferSide(RenderData, VoxelSide::SIDE_NORTH, ChunkRenderCoordinate(0, 1, 0));
	InsertIntoBufferSide(RenderData, VoxelSide::SIDE_NORTH, ChunkRenderCoordinate(1, 1, 0));
	InsertIntoBufferSide(RenderData, VoxelSide::SIDE_NORTH, ChunkRenderCoordinate(1, 0, 0));

	ChunksToRender.Push(RenderData);
}

void ChunkRenderer::DestroyChunkRenderer()
{
	glDeleteVertexArrays(6, &EastVAO);

	for (unsigned int Index = 0; Index < ChunksToRender.GetNum(); ++Index)
	{
		delete ChunksToRender[Index];
	}
	ChunksToRender.Reserve(0);

	delete ChunkRenderShader;

	glDeleteBuffers(12, &EastVBO);
}

#define PI 3.14159265359f

void ChunkRenderer::RenderAllChunks(Player* CurrentPlayer, float CumulativeTime)
{
	ChunkRenderShader->Bind();

	glm::mat4 Projection = glm::perspective(glm::radians(70.0f), 4.0f / 3.0f, 0.1f, 100.f);
	glm::mat4 View = CurrentPlayer->GetViewMatrix();

	ChunkRenderShader->SetProjectionMatrix(Projection);
	ChunkRenderShader->SetViewMatrixLocation(View);

	for (unsigned int Index = 0; Index < ChunksToRender.GetNum(); ++Index)
	{
		if (!ChunksToRender[Index])
			return;

		ChunkRenderShader->SetChunkPositionOffset(ChunksToRender[Index]->ChunkPosition);

		// Render the east face
		glBindVertexArray(EastVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->EastFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ARRAY_BUFFER, EastVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EastIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumEastFaces);

		// Render the west face
		glBindVertexArray(WestVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->WestFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ARRAY_BUFFER, WestVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, WestIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumWestFaces);

		// Render the top face
		glBindVertexArray(TopVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->TopFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ARRAY_BUFFER, TopVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TopIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumTopFaces);

		// Render the bottom face
		glBindVertexArray(BottomVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->BottomFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ARRAY_BUFFER, BottomVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BottomIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumBottomFaces);

		// Render the north face
		glBindVertexArray(NorthVAO);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->NorthFacePBO);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ARRAY_BUFFER, NorthVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NorthIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumNorthFaces);

		// Render the south face
		glBindVertexArray(SouthVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ChunksToRender[Index]->SouthFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 3, GL_UNSIGNED_BYTE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ARRAY_BUFFER, SouthVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SouthIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, ChunksToRender[Index]->NumSouthFaces);
	}
	glBindVertexArray(0);
}

__forceinline void ChunkRenderer::InsertIntoBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate& NewCoordinate)
{
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

__forceinline void ChunkRenderer::InsertBatchIntoBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate* RenderCoords, uint32_t& NumRenderCoords)
{
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
}

void ChunkRenderer::InsertBatchIntoBuffer(GLuint* FacePBO, uint32_t* NumFaces, uint32_t* BufferLength, ChunkRenderCoordinate* RenderCoords, uint32_t& NumRenderCoords)
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

	for (uint32_t Index = 0; Index < NumRenderCoords; ++Index)
	{
		memcpy(BufferStorage + (Index + (*NumFaces)), &RenderCoords[Index].X, 3);
	}
	*NumFaces += NumRenderCoords;
}

__forceinline void ChunkRenderer::SpliceFromBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, ChunkRenderCoordinate& Coordinate)
{
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

void ChunkRenderer::SetData(std::vector<VoxelVertex>& Vertices)
{
}
