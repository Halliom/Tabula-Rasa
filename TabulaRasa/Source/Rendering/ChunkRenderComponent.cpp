#include "ChunkRenderComponent.h"

#include "glm\gtc\matrix_transform.hpp"

// 
GLShaderProgram* ChunkRenderer::ChunkRenderShader = NULL;

// Init the chunk list
std::list<ChunkRenderData*> ChunkRenderer::ChunksToRender;

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

	ChunkRenderer::InsertIntoBuffer(RenderData, VoxelSide::SIDE_EAST, 1, 4, 2);
	ChunkRenderer::InsertIntoBuffer(RenderData, VoxelSide::SIDE_EAST, 1, 3, 5);
	ChunkRenderer::InsertIntoBuffer(RenderData, VoxelSide::SIDE_EAST, 1, 2, 3);
	ChunkRenderer::InsertIntoBuffer(RenderData, VoxelSide::SIDE_EAST, 2, 6, 6);
	ChunkRenderer::InsertIntoBuffer(RenderData, VoxelSide::SIDE_EAST, 7, 6, 2);
	ChunkRenderer::InsertIntoBuffer(RenderData, VoxelSide::SIDE_EAST, 1, 2, 2);
	ChunkRenderer::SpliceFromBuffer(RenderData, VoxelSide::SIDE_EAST, 1, 2, 3);
}

void ChunkRenderer::DestroyChunkRenderer()
{
	glDeleteVertexArrays(6, &EastVAO);

	for (auto& It : ChunksToRender)
	{
		if (It)
		{
			delete It;
		}
	}
	ChunksToRender.clear();

	delete ChunkRenderShader;

	glDeleteBuffers(12, &EastVBO);
}

#define PI 3.14159265359f

void ChunkRenderer::RenderAllChunks(Player* CurrentPlayer, float CumulativeTime)
{
	ChunkRenderShader->Bind();

	glm::mat4 Projection = glm::perspective(glm::radians(70.0f), 4.0f / 3.0f, 0.1f, 100.f);
	glm::mat4 View = CurrentPlayer->GetViewMatrix();
	glm::mat4 Model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -2.0f, -4.0f));

	ChunkRenderShader->SetProjectionMatrix(Projection);
	ChunkRenderShader->SetViewMatrixLocation(View);
	ChunkRenderShader->SetModelMatrix(Model);

	for (auto& It : ChunksToRender)
	{
		// Render the east face
		glBindBuffer(GL_ARRAY_BUFFER, It->EastFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindVertexArray(EastVAO);
		glBindBuffer(GL_ARRAY_BUFFER, EastVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EastIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, It->NumEastFaces);

		// Render the Bottom face
		glBindBuffer(GL_ARRAY_BUFFER, It->BottomFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindVertexArray(BottomVAO);
		glBindBuffer(GL_ARRAY_BUFFER, BottomVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BottomIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, It->NumBottomFaces);

		// Render the top face
		glBindBuffer(GL_ARRAY_BUFFER, It->TopFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindVertexArray(TopVAO);
		glBindBuffer(GL_ARRAY_BUFFER, TopVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TopIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, It->NumTopFaces);

		// Render the bottom face
		glBindBuffer(GL_ARRAY_BUFFER, It->BottomFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindVertexArray(BottomVAO);
		glBindBuffer(GL_ARRAY_BUFFER, BottomVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BottomIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, It->NumBottomFaces);

		// Render the north face
		glBindBuffer(GL_ARRAY_BUFFER, It->NorthFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindVertexArray(NorthVAO);
		glBindBuffer(GL_ARRAY_BUFFER, NorthVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NorthIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, It->NumNorthFaces);

		// Render the south face
		glBindBuffer(GL_ARRAY_BUFFER, It->NorthFacePBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindVertexArray(SouthVAO);
		glBindBuffer(GL_ARRAY_BUFFER, SouthVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SouthIBO);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, It->NumSouthFaces);
	}
}

void ChunkRenderer::InsertIntoBuffer(ChunkRenderData* RenderData, const VoxelSide& Side, uint8_t NewX, uint8_t NewY, uint8_t NewZ)
{
	switch (Side)
	{
		case SIDE_EAST:
		{
			glBindBuffer(GL_ARRAY_BUFFER, RenderData->EastFacePBO);
			uint8_t* BufferStorage = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
			if (RenderData->NumEastFaces < (RenderData->EastFacesBufferLength - 1))
			{
				BufferStorage += (3 * RenderData->NumEastFaces);

				memcpy(BufferStorage, &NewX, 1);
				memcpy(BufferStorage + 1, &NewY, 1);
				memcpy(BufferStorage + 2, &NewZ, 1);
				++RenderData->NumEastFaces;
			}
			else
			{
				glBufferData(GL_ARRAY_BUFFER, RenderData->NumEastFaces + INCREASE_AMOUNT, BufferStorage, 0);
			}
			break;
		}
		case SIDE_WEST :
		{
			glBindBuffer(GL_ARRAY_BUFFER, RenderData->WestFacePBO);
			uint8_t* BufferStorage = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
			if (RenderData->NumWestFaces < (RenderData->WestFacesBufferLength - 1))
			{
				BufferStorage += (3 * RenderData->NumWestFaces);

				memcpy(BufferStorage, &NewX, 1);
				memcpy(BufferStorage + 1, &NewY, 1);
				memcpy(BufferStorage + 2, &NewZ, 1);
				RenderData->NumWestFaces += 1;
			}
			else
			{
				glBufferData(GL_ARRAY_BUFFER, RenderData->NumWestFaces + INCREASE_AMOUNT, BufferStorage, 0);
			}
			break;
		}
		case SIDE_TOP:
		{
			glBindBuffer(GL_ARRAY_BUFFER, RenderData->TopFacePBO);
			uint8_t* BufferStorage = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
			if (RenderData->NumTopFaces < (RenderData->TopFacesBufferLength - 1))
			{
				BufferStorage += (3 * RenderData->NumTopFaces);

				memcpy(BufferStorage, &NewX, 1);
				memcpy(BufferStorage + 1, &NewY, 1);
				memcpy(BufferStorage + 2, &NewZ, 1);
				RenderData->NumTopFaces += 1;
			}
			else
			{
				glBufferData(GL_ARRAY_BUFFER, RenderData->NumTopFaces + INCREASE_AMOUNT, BufferStorage, 0);
			}
			break;
		}
		case SIDE_BOTTOM:
		{
			glBindBuffer(GL_ARRAY_BUFFER, RenderData->BottomFacePBO);
			uint8_t* BufferStorage = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
			if (RenderData->NumBottomFaces < (RenderData->BottomFacesBufferLength - 1))
			{
				BufferStorage += (3 * RenderData->NumBottomFaces);

				memcpy(BufferStorage, &NewX, 1);
				memcpy(BufferStorage + 1, &NewY, 1);
				memcpy(BufferStorage + 2, &NewZ, 1);
				RenderData->NumBottomFaces += 1;
			}
			else
			{
				glBufferData(GL_ARRAY_BUFFER, RenderData->NumBottomFaces + INCREASE_AMOUNT, BufferStorage, 0);
			}
			break;
		}
		case SIDE_NORTH:
		{
			glBindBuffer(GL_ARRAY_BUFFER, RenderData->NorthFacePBO);
			uint8_t* BufferStorage = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
			if (RenderData->NumNorthFaces < (RenderData->NorthFacesBufferLength - 1))
			{
				BufferStorage += (3 * RenderData->NumNorthFaces);

				memcpy(BufferStorage, &NewX, 1);
				memcpy(BufferStorage + 1, &NewY, 1);
				memcpy(BufferStorage + 2, &NewZ, 1);
				RenderData->NumNorthFaces += 1;
			}
			else
			{
				glBufferData(GL_ARRAY_BUFFER, RenderData->NumNorthFaces + INCREASE_AMOUNT, BufferStorage, 0);
			}
			break;
		}
		case SIDE_SOUTH:
		{
			glBindBuffer(GL_ARRAY_BUFFER, RenderData->SouthFacePBO);
			uint8_t* BufferStorage = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
			if (RenderData->NumSouthFaces < (RenderData->SouthFacesBufferLength - 1))
			{
				BufferStorage += (3 * RenderData->NumSouthFaces);

				memcpy(BufferStorage, &NewX, 1);
				memcpy(BufferStorage + 1, &NewY, 1);
				memcpy(BufferStorage + 2, &NewZ, 1);
				RenderData->NumSouthFaces += 1;
			}
			else
			{
				glBufferData(GL_ARRAY_BUFFER, RenderData->NumSouthFaces + INCREASE_AMOUNT, BufferStorage, 0);
			}
			break;
		}
		default: { break; }
	}
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

__forceinline void ChunkRenderer::SpliceFromBufferSide(ChunkRenderData* RenderData, const VoxelSide& Side, uint8_t X, uint8_t Y, uint8_t Z)
{
	switch (Side)
	{
		case SIDE_EAST:
		{
			SpliceFromBuffer(&RenderData->EastFacePBO, &RenderData->NumEastFaces, &RenderData->EastFacesBufferLength, X, Y, Z);
			break;
		}
		case SIDE_WEST:
		{
			SpliceFromBuffer(&RenderData->WestFacePBO, &RenderData->NumWestFaces, &RenderData->WestFacesBufferLength, X, Y, Z);
			break;
		}
		case SIDE_TOP:
		{
			SpliceFromBuffer(&RenderData->TopFacePBO, &RenderData->NumTopFaces, &RenderData->TopFacesBufferLength, X, Y, Z);
			break;
		}
		case SIDE_BOTTOM:
		{
			SpliceFromBuffer(&RenderData->BottomFacePBO, &RenderData->NumBottomFaces, &RenderData->BottomFacesBufferLength, X, Y, Z);
			break;
		}
		case SIDE_NORTH:
		{
			SpliceFromBuffer(&RenderData->NorthFacePBO, &RenderData->NumNorthFaces, &RenderData->NorthFacesBufferLength, X, Y, Z);
			break;
		}
		case SIDE_SOUTH:
		{
			SpliceFromBuffer(&RenderData->SouthFacePBO, &RenderData->NumSouthFaces, &RenderData->SouthFacesBufferLength, X, Y, Z);
			break;
		}
		default: { break; }
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void ChunkRenderer::SpliceFromBuffer(GLuint * FacePBO, uint32_t * NumFaces, uint32_t * BufferLength, uint8_t & X, uint8_t & Y, uint8_t & Z)
{
	glBindBuffer(GL_ARRAY_BUFFER, *FacePBO);

	if (*NumFaces == 0) // Nothing to do
		return;

	if (*NumFaces == 1)
		glBufferData(GL_ARRAY_BUFFER, INITIAL_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW); // Just reset it

	uint8_t* BufferStorage = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	unsigned int Index = 0;
	do {
		if (X == BufferStorage[0] &&
			Y == BufferStorage[1] &&
			Z == BufferStorage[2])
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
