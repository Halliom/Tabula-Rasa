#pragma once

#include "VoxelLibPluginPrivatePCH.h"

#include "VoxelMeshComponent.h"

#include "SolidActor.generated.h"

USTRUCT(BlueprintType)
struct FWorldPosition
{
	GENERATED_BODY()
public:
	
	UPROPERTY(Category="Position", EditDefaultsOnly)
	int32 PositionX;

	UPROPERTY(Category = "Position", EditDefaultsOnly)
	int32 PositionY;

	UPROPERTY(Category = "Position", EditDefaultsOnly)
	int32 PositionZ;
};

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
	static void SpawnSolid(UObject* Creator, FWorldPosition SpawnPosition);

public:

	ASolidActor(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(Category="Solid", BlueprintCallable)
	FWorldPosition GetWorldPosition()
	{
		return WorldPosition;
	}

private:

	UPROPERTY()
	UVoxelMeshComponent* Voxel;

	UPROPERTY()
	FWorldPosition WorldPosition;

};