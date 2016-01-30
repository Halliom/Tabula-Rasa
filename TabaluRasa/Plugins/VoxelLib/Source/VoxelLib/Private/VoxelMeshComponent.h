#pragma once

#include "VoxelLibPluginPrivatePCH.h"
#include "DynamicMeshBuilder.h"

#include "VoxelMeshComponent.generated.h"

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
};

UCLASS(meta=(BlueprintSpawnableComponent))
class UVoxelMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

	UVoxelMeshComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) 
	{
		MarkRenderStateDirty();
	}

private:

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual int32 GetNumMaterials() const override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
};