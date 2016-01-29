#include "VoxelLibPluginPrivatePCH.h"

#include "VoxelMeshComponent.h"

FPrimitiveSceneProxy* UVoxelMeshComponent::CreateSceneProxy()
{
	//TODO: Implementation
	return 0;
}

int32 UVoxelMeshComponent::GetNumMaterials() const
{
	//TODO: Implementation
	return 0;
}

FBoxSphereBounds UVoxelMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	//TODO: Implementation
	return FBoxSphereBounds();
}

inline void FVoxelVertexFactory::InitVertexComponentsRenderThread(const FVoxelVertexBuffer* VertexBuffer)
{
	// From FLocalVertexFactory, just a data class that contains
	// position-, texture- and color-components for the vertex factory
	DataType NewData;

	NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Position, VET_Float3);
	NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentX, VET_PackedNormal);
	NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentZ, VET_PackedNormal);
	NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Color, VET_Color);

	NewData.TextureCoordinates.Add(STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TextureCoordinate, VET_Float2));

	SetData(NewData);
}

inline void FVoxelVertexFactory::InitVertexComponentsGameThread(const FVoxelVertexBuffer* VertexBuffer)
{
	if (IsInRenderingThread())
	{
		// We already are in the render thread, send it to the render thread
		// version of this function
		InitVertexComponentsRenderThread(VertexBuffer);
	}
	else
	{
		// Send this and the vertex buffer to the rendering thread to be initialized
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(	VertexFactoryInitRenderThread,	// Name of function
													const FVoxelVertexBuffer*,		// Type of parameter 1
													VertexBuffer,					// Name of parameter 1 (inside code block)
													VertexBuffer,					// Value of parameter 1
													FVoxelVertexFactory*,			// Type of parameter 2
													VertexFactory,					// Name of parameter 2 (inside code block)
													this,							// Value of parameter 2
		{
			VertexFactory->InitVertexComponentsRenderThread(VertexBuffer);
		});
	}
}


// This should be used when adding shader
//IMPLEMENT_VERTEX_FACTORY_TYPE(FVoxelVertexFactory, "VoxelVertexFactory", true, true, true, true, true);
