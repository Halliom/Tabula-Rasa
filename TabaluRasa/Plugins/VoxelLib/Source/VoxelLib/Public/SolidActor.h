#pragma once

#include "VoxelLibPluginPrivatePCH.h"

#include "VoxelMeshComponent.h"

#include "SolidActor.generated.h"

UCLASS(Blueprintable)
class VOXELLIB_API ASolidActor : public AActor
{
	GENERATED_BODY()
public:

	UFUNCTION(Category = "Position", BlueprintCallable)
	static FWorldPosition CreateWorldPosition(int32 PositionX, int32 PositionY, int32 PositionZ)
	{
		return { PositionX, PositionY, PositionZ };
	}

	UFUNCTION(Category = "Solid", BlueprintCallable)
	static ASolidActor* SpawnSolidAt(AActor* ParentActor, FWorldPosition SpawnPosition);
	
	UFUNCTION(Category = "Solid", BlueprintCallable)
	static ASolidActor* SpawnSolid(AActor* ParentActor, int32 X, int32 Y, int32 Z)
	{
		return ASolidActor::SpawnSolidAt(ParentActor, FWorldPosition(X, Y, Z));
	}

	UFUNCTION(Category = "Solid", BlueprintCallable)
	static ASolidActor* GetSolidAtLocation(FWorldPosition& Position);

public:

	ASolidActor(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(Category="Solid", BlueprintCallable)
	FWorldPosition GetWorldPosition()
	{
		return WorldPosition;
	}

	void OnNodePlacedAdjacent(const TArray<ASolidActor*>& SurroundingNodes);

	void OnNodePlacedOnSide(const EVoxelSide& Side);

	UPROPERTY()
	FWorldPosition WorldPosition;

	UPROPERTY()
	FWorldPosition LocalChunkPosition;

	AChunk* Chunk;

private:

	UPROPERTY()
	UVoxelMeshComponent* Voxel;

};