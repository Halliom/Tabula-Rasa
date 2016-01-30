#include "VoxelLibPluginPrivatePCH.h"

#include "VoxelMeshComponent.h"

FPrimitiveSceneProxy* UVoxelMeshComponent::CreateSceneProxy()
{
	GEngine->AddOnScreenDebugMessage(1, 5.0f,FColor::Red, TEXT("Testing"));
	println("CreateSceneProxy");
	return new FVoxelSceneProxy(this);
}

int32 UVoxelMeshComponent::GetNumMaterials() const
{
	return 1;
}

FBoxSphereBounds UVoxelMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	//TODO: Implementation
	FBoxSphereBounds NewBounds;
	NewBounds.Origin = FVector::ZeroVector;
	NewBounds.BoxExtent = FVector(HALF_WORLD_MAX, HALF_WORLD_MAX, HALF_WORLD_MAX);
	NewBounds.SphereRadius = FMath::Sqrt(3.0f * FMath::Square(HALF_WORLD_MAX));
	return NewBounds;
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

#define CUBE_SIZE 50.0f

struct Triangle
{
	FVector Vertex0;
	FVector Vertex1;
	FVector Vertex2;

	Triangle(FVector V0, FVector V1, FVector V2) : Vertex0(V0), Vertex1(V1), Vertex2(V2) {}
};

FVoxelSceneProxy::FVoxelSceneProxy(UVoxelMeshComponent* Component) : 
	FPrimitiveSceneProxy(Component),
	MaterialRelevance(Component->GetMaterialRelevance(ERHIFeatureLevel::SM4))
{
	//-------Bottom level-------

	FDynamicMeshVertex Vertex0;
	Vertex0.Position = FVector(0, CUBE_SIZE, 0.0f);
	Vertex0.Color = FColor(255, 255, 255);
	//Vertex0.SetTangents(FVector(-1, 0, 0), FVector(0, 1, 0), FVector(0, 0, 1));
	VertexBuffer.Vertices.Add(Vertex0);

	FDynamicMeshVertex Vertex1;
	Vertex1.Position = FVector(0, 0, 0.0f);
	Vertex1.Color = FColor(255, 255, 255);
	//Vertex1.SetTangents(FVector(-1, 0, 0), FVector(0, -1, 0), FVector(0, 0, 1));
	VertexBuffer.Vertices.Add(Vertex1);

	FDynamicMeshVertex Vertex2;
	Vertex2.Position = FVector(CUBE_SIZE, CUBE_SIZE, 0.0f);
	Vertex2.Color = FColor(255, 255, 255);
	//Vertex2.SetTangents(FVector(1, 0, 0), FVector(0, 1, 0), FVector(0, 0, 1));
	VertexBuffer.Vertices.Add(Vertex2);

	FDynamicMeshVertex Vertex3;
	Vertex3.Position = FVector(CUBE_SIZE, 0, 0.0f);
	Vertex3.Color = FColor(255, 255, 255);
	//Vertex3.SetTangents(FVector(1, 0, 0), FVector(0, -1, 0), FVector(0, 0, 1));
	VertexBuffer.Vertices.Add(Vertex3);
	
	//-------Top level-------

	FDynamicMeshVertex Vertex4;
	Vertex4.Position = FVector(0, CUBE_SIZE, CUBE_SIZE);
	Vertex4.Color = FColor(255, 255, 255);
	//Vertex4.SetTangents(FVector(-1, 0, 0), FVector(0, 1, 0), FVector(0, 0, -1));
	VertexBuffer.Vertices.Add(Vertex4);

	FDynamicMeshVertex Vertex5;
	Vertex5.Position = FVector(0, 0, CUBE_SIZE);
	Vertex5.Color = FColor(255, 255, 255);
	//Vertex5.SetTangents(FVector(-1, 0, 0), FVector(0, -1, 0), FVector(0, 0, -1));
	VertexBuffer.Vertices.Add(Vertex5);

	FDynamicMeshVertex Vertex6;
	Vertex6.Position = FVector(CUBE_SIZE, CUBE_SIZE, CUBE_SIZE);
	Vertex6.Color = FColor(255, 255, 255);
	//Vertex6.SetTangents(FVector(1, 0, 0), FVector(0, 1, 0), FVector(0, 0, -1));
	VertexBuffer.Vertices.Add(Vertex6);

	FDynamicMeshVertex Vertex7;
	Vertex7.Position = FVector(CUBE_SIZE, 0, CUBE_SIZE);
	Vertex7.Color = FColor(255, 255, 255);
	//Vertex7.SetTangents(FVector(1, 0, 0), FVector(0, -1, 0), FVector(0, 0, -1));
	VertexBuffer.Vertices.Add(Vertex7);

	// BOTTOM FACE draw clockwise
	IndexBuffer.Indices.Add(0); IndexBuffer.Indices.Add(1); IndexBuffer.Indices.Add(3);
	IndexBuffer.Indices.Add(0); IndexBuffer.Indices.Add(3); IndexBuffer.Indices.Add(2);

	// TOP FACE draw anti-clockwise so it faces outwards
	IndexBuffer.Indices.Add(7); IndexBuffer.Indices.Add(5); IndexBuffer.Indices.Add(4);
	IndexBuffer.Indices.Add(6); IndexBuffer.Indices.Add(7); IndexBuffer.Indices.Add(4);

	// FRONT FACE
	IndexBuffer.Indices.Add(0); IndexBuffer.Indices.Add(2); IndexBuffer.Indices.Add(4);
	IndexBuffer.Indices.Add(2); IndexBuffer.Indices.Add(6); IndexBuffer.Indices.Add(4);

	// BACK FACE
	IndexBuffer.Indices.Add(1); IndexBuffer.Indices.Add(5); IndexBuffer.Indices.Add(3);
	IndexBuffer.Indices.Add(3); IndexBuffer.Indices.Add(5); IndexBuffer.Indices.Add(7);

	// LEFT FACE
	IndexBuffer.Indices.Add(5); IndexBuffer.Indices.Add(1); IndexBuffer.Indices.Add(0);
	IndexBuffer.Indices.Add(4); IndexBuffer.Indices.Add(5); IndexBuffer.Indices.Add(0);

	// RIGHT FACE
	IndexBuffer.Indices.Add(2); IndexBuffer.Indices.Add(3); IndexBuffer.Indices.Add(6);
	IndexBuffer.Indices.Add(7); IndexBuffer.Indices.Add(6); IndexBuffer.Indices.Add(3);

	VertexFactory.InitVertexComponentsGameThread(&VertexBuffer);

	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&VertexFactory);

	// Get first material
	Material = Component->GetMaterial(0);
	if (Material == NULL)
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
	return ViewRelevance;
}
