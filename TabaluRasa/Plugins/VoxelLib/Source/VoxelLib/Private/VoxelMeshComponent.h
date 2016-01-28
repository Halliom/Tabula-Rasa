#pragma once

#include "VoxelLibPluginPrivatePCH.h"

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
	}
};



UCLASS(meta=(BlueprintSpawnableComponent)
class UVoxelMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

private:

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual int32 GetNumMaterials() const override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
};