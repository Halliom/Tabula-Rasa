#include "ChunkRenderComponent.h"

#include "glm\gtc\matrix_transform.hpp"

// 
GLShaderProgram* ChunkRenderer::ChunkRenderShader = NULL;

// Init the chunk list
std::list<ChunkRenderData*> ChunkRenderer::ChunksToRender;

// Init the Vertex Array Object to be 0 (uninitialized)
GLuint ChunkRenderer::EastVAO = 0;
GLuint ChunkRenderer::WestVAO = 0;
GLuint ChunkRenderer::TopVAO = 0;
GLuint ChunkRenderer::BottomVAO = 0;
GLuint ChunkRenderer::NorthVAO = 0;
GLuint ChunkRenderer::SouthVAO = 0;

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

	glBindVertexArray(0);
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

	glBindVertexArray(EastVAO);
	glBindBuffer(GL_ARRAY_BUFFER, EastVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EastIBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

	glBindVertexArray(WestVAO);
	glBindBuffer(GL_ARRAY_BUFFER, WestVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, WestIBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

	glBindVertexArray(TopVAO);
	glBindBuffer(GL_ARRAY_BUFFER, TopVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TopIBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	
	glBindVertexArray(BottomVAO);
	glBindBuffer(GL_ARRAY_BUFFER, BottomVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BottomIBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

	glBindVertexArray(NorthVAO);
	glBindBuffer(GL_ARRAY_BUFFER, NorthVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NorthIBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

	glBindVertexArray(SouthVAO);
	glBindBuffer(GL_ARRAY_BUFFER, SouthVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SouthIBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
}

void ChunkRenderer::SetData(std::vector<VoxelVertex>& Vertices)
{
}
