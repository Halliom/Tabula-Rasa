#include "ChunkRenderComponent.h"

ChunkRenderComponent::ChunkRenderComponent() :
	NumVertices(0)
{
	glGenVertexArrays(1, &VertexArrayObject);
	glGenBuffers(1, &VertexBufferObject);
}

ChunkRenderComponent::~ChunkRenderComponent()
{
	glDeleteVertexArrays(1, &VertexArrayObject);
	glDeleteBuffers(1, &VertexBufferObject);
}

void ChunkRenderComponent::Render(float DeltaTime)
{
	glBindVertexArray(VertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*) offsetof(VoxelVertex, Position));
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(VoxelVertex), (void*) offsetof(VoxelVertex, ColorRed));

	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

void ChunkRenderComponent::SetData(std::vector<VoxelVertex>& Vertices)
{
	glBindVertexArray(VertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VoxelVertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	NumVertices = Vertices.size();
}
