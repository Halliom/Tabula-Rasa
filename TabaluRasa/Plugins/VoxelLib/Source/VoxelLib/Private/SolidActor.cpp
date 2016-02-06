#include "VoxelLibPluginPrivatePCH.h"

#include "ChunkManager.h"
#include "SolidActor.h"

ASolidActor* ASolidActor::SpawnSolid(UObject* Creator, FWorldPosition SpawnPosition)
{
	// Get the chunk of the spawn position
	AChunk* Chunk = AChunkManager::GetStaticChunkManager()->GetOrCreateChunkFromWorldPosition(SpawnPosition);
	if (Chunk)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(FVector(SpawnPosition.PositionX * 50.0f, SpawnPosition.PositionY * 50.0f, SpawnPosition.PositionZ * 50.0f + 25.0f));
		ASolidActor* SpawnedActor = (ASolidActor*)Creator->GetWorld()->SpawnActor(ASolidActor::StaticClass(), &SpawnTransform);

		SpawnedActor->WorldPosition = SpawnPosition;

		FWorldPosition LocalPosition = FWorldPosition(
			SpawnPosition.PositionX % INITIAL_CHUNK_SIZE,
			SpawnPosition.PositionY % INITIAL_CHUNK_SIZE,
			SpawnPosition.PositionZ % INITIAL_CHUNK_SIZE);
		Chunk->InsertIntoChunk(LocalPosition, SpawnedActor);

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

void ASolidActor::BeginPlay()
{
	Super::BeginPlay();

	//TODO: This check should not have to be performed
	if (ContainingNode)
	{
		Voxel->SetSidesToRender(ContainingNode->Chunk->GetRenderFaceMask(LocalChunkPosition));
	}
}

void ASolidActor::OnNodePlacedAdjacent()
{
	//TODO: This check should not have to be performed
	if (ContainingNode)
	{
		Voxel->SetSidesToRender(ContainingNode->Chunk->GetRenderFaceMask(LocalChunkPosition));
	}
}