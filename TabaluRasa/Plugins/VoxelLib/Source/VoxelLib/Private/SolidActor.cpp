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

	//static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/BlankMaterial.BlankMaterial'"));
	
	//Voxel->SetMaterial(0, Material.Object);
}

FORCEINLINE void ASolidActor::OnNodePlacedAdjacent(const TArray<ASolidActor*>& SurroundingNodes)
{
	uint32 Result = 63;
	if (SurroundingNodes[0])
	{
		Result ^= EVoxelSide::VS_SIDE_TOP;
		SurroundingNodes[0]->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_BOTTOM);
	}
	if (SurroundingNodes[1])
	{
		Result ^= EVoxelSide::VS_SIDE_BOTTOM;
		SurroundingNodes[1]->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_TOP);
	}
	if (SurroundingNodes[2])
	{
		Result ^= EVoxelSide::VS_SIDE_FRONT;
		SurroundingNodes[2]->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_BACK);
	}
	if (SurroundingNodes[3])
	{
		Result ^= EVoxelSide::VS_SIDE_BACK;
		SurroundingNodes[3]->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_FRONT);
	}
	if (SurroundingNodes[4])
	{
		Result ^= EVoxelSide::VS_SIDE_LEFT;
		SurroundingNodes[4]->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_RIGHT);
	}
	if (SurroundingNodes[5])
	{
		Result ^= EVoxelSide::VS_SIDE_RIGHT;
		SurroundingNodes[5]->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_LEFT);
	}
	Voxel->SetSidesToRender(Result);
}

FORCEINLINE void ASolidActor::OnNodePlacedOnSide(const EVoxelSide& Side)
{
	Voxel->RemoveSideToRender(Side);
}