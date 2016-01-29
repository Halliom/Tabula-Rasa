#include "VoxelLibPluginPrivatePCH.h"

#include "VoxelMeshComponent.h"

FPrimitiveSceneProxy* UVoxelMeshComponent::CreateSceneProxy()
{
	return new FVoxelSceneProxy(this);
}

int32 UVoxelMeshComponent::GetNumMaterials() const
{
	return 1;
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

#define CUBE_SIZE = 50.0f

FVoxelSceneProxy::FVoxelSceneProxy(UVoxelMeshComponent* Component) : 
	FPrimitiveSceneProxy(Component),
	MaterialRelevance(Component->GetMaterialRelevance(ERHIFeatureLevel::SM4))
{
	//TODO: Add vertices
	//VertexBuffer.Vertices.Add();

	FDynamicMeshVertex Vertex0;
	Vertex0.Position = FVector(CUBE_SIZE / -2.0f, CUBE_SIZE / 2.0f, 0.0f);
	VertexBuffer.Vertices.Add(Vertex0);

	FDynamicMeshVertex Vertex1;
	Vertex1.Position = FVector(CUBE_SIZE / -2.0f, CUBE_SIZE / -2.0f, 0.0f);
	VertexBuffer.Vertices.Add(Vertex1);

	FDynamicMeshVertex Vertex2;
	Vertex2.Position = FVector(CUBE_SIZE / 2.0f, CUBE_SIZE / 2.0f, 0.0f);
	VertexBuffer.Vertices.Add(Vertex2);

	FDynamicMeshVertex Vertex3;
	Vertex3.Position = FVector(CUBE_SIZE / 2.0f, CUBE_SIZE / -2.0f, 0.0f);
	VertexBuffer.Vertices.Add(Vertex3);
	
	//-------Top level-------

	FDynamicMeshVertex Vertex4;
	Vertex4.Position = FVector(CUBE_SIZE / -2.0f, CUBE_SIZE / 2.0f, CUBE_SIZE);
	VertexBuffer.Vertices.Add(Vertex4);

	FDynamicMeshVertex Vertex5;
	Vertex5.Position = FVector(CUBE_SIZE / -2.0f, CUBE_SIZE / -2.0f, CUBE_SIZE);
	VertexBuffer.Vertices.Add(Vertex5);

	FDynamicMeshVertex Vertex6;
	Vertex6.Position = FVector(CUBE_SIZE / 2.0f, CUBE_SIZE / 2.0f, CUBE_SIZE);
	VertexBuffer.Vertices.Add(Vertex6);

	FDynamicMeshVertex Vertex7;
	Vertex7.Position = FVector(CUBE_SIZE / 2.0f, CUBE_SIZE / -2.0f, CUBE_SIZE);
	VertexBuffer.Vertices.Add(Vertex7);

	//IndexBuffer.Indices.Add();

	VertexFactory.InitVertexComponentsGameThread(&VertexBuffer);

	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&VertexFactory);

	// Get first material
	Material = Component->GetMaterial(0);
	if (!Material)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}
}

FVoxelSceneProxy::~FVoxelSceneProxy()
{
	// Game thread telling the buffers and factory to start destroying themselves
	VertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
}

void FVoxelSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const
{
	FMaterialRenderProxy* MaterialProxy = NULL;

	MaterialProxy = Material->GetRenderProxy(IsSelected());

	// Add a "mesh batch" to the collector which collects different meshes for different views
	// to be rendered.
	for (int i = 0; i < Views.Num(); ++i)
	{
		if (VisibilityMap & (1 << i))
		{
			const FSceneView* View = Views[i];
			
			FMeshBatch& Mesh = Collector.AllocateMesh();
			FMeshBatchElement& BatchElement = Mesh.Elements[0];

			Mesh.VertexFactory = &VertexFactory;
			Mesh.MaterialRenderProxy = MaterialProxy;
			Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
			Mesh.Type = PT_TriangleList; // Draw triangles
			Mesh.DepthPriorityGroup = SDPG_World;
			Mesh.bCanApplyViewModeOverrides = false;

			BatchElement.IndexBuffer = &IndexBuffer;
			BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
			BatchElement.FirstIndex = 0;
			BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3; // Triangles have 3 points, 
			BatchElement.MinVertexIndex = 0;
			BatchElement.MaxVertexIndex = VertexBuffer.Vertices.Num() - 1;

			Collector.AddMesh(i, Mesh);
		}
	}
}

FPrimitiveViewRelevance FVoxelSceneProxy::GetViewRelevance(const FSceneView * View)
{
	FPrimitiveViewRelevance ViewRelevance;
	ViewRelevance.bDrawRelevance = IsShown(View);
	ViewRelevance.bShadowRelevance = IsShadowCast(View);
	ViewRelevance.bDynamicRelevance = true;
	ViewRelevance.bRenderInMainPass = ShouldRenderInMainPass();
	ViewRelevance.bRenderCustomDepth = ShouldRenderCustomDepth();
	MaterialRelevance.SetPrimitiveViewRelevance(ViewRelevance);
	return FPrimitiveViewRelevance();
}
