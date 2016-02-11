#include "VoxelLibPluginPrivatePCH.h"

#include "ChunkManager.h"
#include "SolidActor.h"

ASolidActor* ASolidActor::SpawnSolidAt(AActor* ParentActor, FWorldPosition SpawnPosition)
{
	// Get the chunk of the spawn position
	AChunk* Chunk = AChunkManager::GetStaticChunkManager()->GetOrCreateChunkFromWorldPosition(ParentActor, SpawnPosition);

	//TODO: This check should not have to be performed
	if (Chunk)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(FVector(SpawnPosition.PositionX * 50.0f, SpawnPosition.PositionY * 50.0f, SpawnPosition.PositionZ * 50.0f));
		ASolidActor* SpawnedActor = Cast<ASolidActor>(ParentActor->GetWorld()->SpawnActor(ASolidActor::StaticClass(), &SpawnTransform));

		SpawnedActor->WorldPosition = SpawnPosition;

		FWorldPosition LocalPosition = FWorldPosition(
			SpawnPosition.PositionX % INITIAL_CHUNK_SIZE,
			SpawnPosition.PositionY % INITIAL_CHUNK_SIZE,
			SpawnPosition.PositionZ % INITIAL_CHUNK_SIZE);
		Chunk->InsertNode(LocalPosition, SpawnedActor);

		return SpawnedActor;
	}
	return NULL;
}

ASolidActor* ASolidActor::GetSolidAtLocation(FWorldPosition& Position)
{
	return NULL;
}

ASolidActor::ASolidActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Voxel = ObjectInitializer.CreateDefaultSubobject<UVoxelMeshComponent>(this, TEXT("Voxel"));
	RootComponent = Voxel;

	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/BlankMaterial.BlankMaterial'"));
	
	Voxel->SetMaterial(0, Material.Object);
}

FORCEINLINE void ASolidActor::OnNodePlacedAdjacent()
{
	Voxel->SetSidesToRender(Chunk->GetRenderFaceMask(LocalChunkPosition));
}

FORCEINLINE void ASolidActor::OnNodePlacedOnSide(const EVoxelSide& Side)
{
	Voxel->RemoveSideToRender(Side);
}