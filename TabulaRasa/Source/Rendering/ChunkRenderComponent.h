#pragma once

#include "RenderComponent.h"
#include "../Engine/Octree.h"

struct VoxelVertex;

class ChunkRenderComponent : public RenderComponent
{
public:

	ChunkRenderComponent();

	virtual ~ChunkRenderComponent();

	virtual void Render(float DeltaTime) override;

	void SetData(std::vector<VoxelVertex>& Vertices);

private:

	GLuint VertexArrayObject;

	GLuint VertexBufferObject;

	uint32_t NumVertices;
};