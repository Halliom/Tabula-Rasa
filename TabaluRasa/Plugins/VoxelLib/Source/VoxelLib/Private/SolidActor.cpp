#include "VoxelLibPluginPrivatePCH.h"

#include "SolidActor.h"

void ASolidActor::SpawnSolid(UObject* Creator, FWorldPosition SpawnPosition)
{
	FTransform SpawnTransform;
	SpawnTransform.TransformPosition(FVector(SpawnPosition.PositionX * 50.0f, SpawnPosition.PositionY * 50.0f, SpawnPosition.PositionZ * 50.0f));
	Creator->GetWorld()->SpawnActor(ASolidActor::StaticClass());

	//TODO: Return the spawned actor?
}

ASolidActor::ASolidActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Voxel = ObjectInitializer.CreateDefaultSubobject<UVoxelMeshComponent>(this, TEXT("Voxel"));

	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/BlankMaterial.BlankMaterial'"));
	
	Voxel->SetMaterial(0, Material.Object);
}