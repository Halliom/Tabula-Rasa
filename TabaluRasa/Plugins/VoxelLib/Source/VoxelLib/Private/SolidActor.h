#pragma once

#include "VoxelLibPluginPrivatePCH.h"

#include "VoxelMeshComponent.h"

#include "SolidActor.generated.h"

UCLASS(Blueprintable)
class ASolidActor : public AActor
{
	GENERATED_BODY()
public:

	UFUNCTION(Category = "Position", BlueprintCallable)
	static FWorldPosition CreateWorldPosition(int32 PositionX, int32 PositionY, int32 PositionZ)
	{
		return { PositionX, PositionY, PositionZ };
	}

	UFUNCTION(Category = "Solid", BlueprintCallable)
	static ASolidActor* SpawnSolid(UObject* Creator, FWorldPosition SpawnPosition);

	UFUNCTION(Category = "Solid", BlueprintCallable)
	static ASolidActor* GetSolidAtLocation(FWorldPosition& Position);

public:

	ASolidActor(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(Category="Solid", BlueprintCallable)
	FWorldPosition GetWorldPosition()
	{
		return WorldPosition;
	}

	virtual void BeginPlay() override;

	void OnNodePlacedAdjacent();

	UPROPERTY()
	FWorldPosition WorldPosition;

	UPROPERTY()
	FWorldPosition LocalChunkPosition;

	AOctreeNode* ContainingNode;

private:

	UPROPERTY()
	UVoxelMeshComponent* Voxel;

};