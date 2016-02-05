#pragma once

#include "VoxelLibPluginPrivatePCH.h"
#include "DynamicMeshBuilder.h"

#include "VoxelMeshComponent.generated.h"

#define CUBE_SIZE 50.0f

/**
 * Defines sides of a solid that the API and game use to determine
 * things such as what side to render or where a solid was clicked
 */
enum EVoxelSide : unsigned int
{
	VS_SIDE_TOP = 1,
	VS_SIDE_BOTTOM = 2,
	VS_SIDE_FRONT = 4,
	VS_SIDE_BACK = 8,
	VS_SIDE_LEFT = 16,
	VS_SIDE_RIGHT = 32,
};

/**
 * This is the vertex buffer that gets sent to the shader to be processed
 * this is where the vertices for the voxel (cube) will be stored
 */
class FVoxelVertexBuffer : public FVertexBuffer
{
public:
	TArray<FDynamicMeshVertex> Vertices;

	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo CreateInfo;
		void* VertexBufferData = 0;
		VertexBufferRHI = RHICreateAndLockVertexBuffer(Vertices.Num() * sizeof(FDynamicMeshVertex), BUF_Static, CreateInfo, VertexBufferData);

		FMemory::Memcpy(VertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(FDynamicMeshVertex));
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
};

/**
 * This is the index buffer that gets sent to the shader to be processed
 * this is where the indices for the vertices for the voxel will be stored
 */
class FVoxelIndexBuffer : public FIndexBuffer
{
public:
	TArray<int32> Indices;

	virtual void InitRHI() override
	{
		// This is sort of like doing buffer mapping in OpenGL, first you tell the
		// Rendering Hardware Interface (RHI) to create a buffer and then maps it to
		// IndexBufferData and then you memcopy the data into that memory and then you
		// unlock the buffer again (the buffer is a local variable from FIndexBuffer).
		FRHIResourceCreateInfo CreateInfo;
		void* IndexBufferData = 0;
		IndexBufferRHI = RHICreateAndLockIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), BUF_Static, CreateInfo, IndexBufferData);

		FMemory::Memcpy(IndexBufferData, Indices.GetData(), Indices.Num() * sizeof(int32));
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
};

class FVoxelVertexFactory : public FLocalVertexFactory
{
	// Only used in conjunction with shader
	//DECLARE_VERTEX_FACTORY_TYPE(FVoxelVertexFactory)
public:

	FVoxelVertexFactory() {}

	inline void InitVertexComponentsRenderThread(const FVoxelVertexBuffer* VertexBuffer);

	inline void InitVertexComponentsGameThread(const FVoxelVertexBuffer* VertexBuffer);
};

class UVoxelMeshComponent;

class FVoxelSceneProxy : public FPrimitiveSceneProxy
{
public:

	FVoxelSceneProxy(UVoxelMeshComponent* Component);

	virtual ~FVoxelSceneProxy();

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) override;

	virtual bool CanBeOccluded() const override { return !MaterialRelevance.bDisableDepthTest; }

	virtual uint32 GetMemoryFootprint() const { return sizeof(*this) + FPrimitiveSceneProxy::GetAllocatedSize(); }

private:
	UMaterialInterface* Material;
	FVoxelVertexBuffer VertexBuffer;
	FVoxelIndexBuffer IndexBuffer;
	FVoxelVertexFactory VertexFactory;

	FMaterialRelevance MaterialRelevance;

	static TArray<FDynamicMeshVertex, TInlineAllocator<8>> StandardVertices;
	static TArray<int32, TInlineAllocator<6>> TopFaceIndices;
	static TArray<int32, TInlineAllocator<6>> BottomFaceIndices;
	static TArray<int32, TInlineAllocator<6>> FrontFaceIndices;
	static TArray<int32, TInlineAllocator<6>> BackFaceIndices;
	static TArray<int32, TInlineAllocator<6>> LeftFaceIndices;
	static TArray<int32, TInlineAllocator<6>> RightFaceIndices;

	static bool bHasBeenInitialized;
};

UCLASS(meta=(BlueprintSpawnableComponent))
class UVoxelMeshComponent : public UMeshComponent
{
	GENERATED_BODY()
	friend class FVoxelSceneProxy;
public:
	UVoxelMeshComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) 
	{
		// Set it to render all sides
		SidesToRender = VS_SIDE_TOP | VS_SIDE_BOTTOM | VS_SIDE_FRONT | VS_SIDE_BACK | VS_SIDE_LEFT | VS_SIDE_RIGHT; //63
	}

	void SetSidesToRender(unsigned int RenderInfo)
	{
		SidesToRender = RenderInfo;
		MarkRenderStateDirty();
	}

private:

	unsigned int SidesToRender;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual int32 GetNumMaterials() const override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
};