#pragma once

#include "VoxelLibPluginPrivatePCH.h"
#include "DynamicMeshBuilder.h"

#include "VoxelMeshComponent.generated.h"

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

UCLASS(meta=(BlueprintSpawnableComponent))
class UVoxelMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

private:

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual int32 GetNumMaterials() const override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
};