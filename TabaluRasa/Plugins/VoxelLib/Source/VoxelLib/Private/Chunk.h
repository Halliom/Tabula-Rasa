#pragma once

#include "VoxelLibPluginPrivatePCH.h"

#include "Chunk.generated.h"

USTRUCT(BlueprintType)
struct FWorldPosition
{
	GENERATED_BODY()
public:

	FWorldPosition(int32 PositionX, int32 PositionY, int32 PositionZ)
		: PositionX(PositionX), PositionY(PositionY), PositionZ(PositionZ)
	{}

	FWorldPosition() {}

	UPROPERTY(Category = "Position", EditDefaultsOnly)
	int32 PositionX;

	UPROPERTY(Category = "Position", EditDefaultsOnly)
	int32 PositionY;

	UPROPERTY(Category = "Position", EditDefaultsOnly)
	int32 PositionZ;

	bool IsWithinBounds(const FWorldPosition& Bounds) const;
};

UCLASS() //TODO: Should this be marked abstract?
class AOctreeNode : public AActor
{
	friend class AChunk;
	GENERATED_BODY()
public:

	AOctreeNode(const FObjectInitializer& ObjectInitializer);

	void InsertNode(const FWorldPosition& LocalPosition);

	virtual void OnNodePlacedAdjacent();

private: 

	AOctreeNode* ParentNode;

	TArray<AOctreeNode*, TInlineAllocator<8>> Children;

protected:

	UPROPERTY()
	FWorldPosition WorldPosition;

	UPROPERTY()
	FWorldPosition LocalPosition;

	UPROPERTY()
	AChunk* Chunk;

};

UCLASS()
class AChunk : public AActor
{
	GENERATED_BODY()
public:

	AChunk(const FObjectInitializer& ObjectInitializer);

	AOctreeNode* GetNodeFromTreeLocal(const FWorldPosition& LocalTreePosition);

	void InsertIntoChunkLocal(const FWorldPosition& LocalTreePosition, AOctreeNode* Node);

	TArray<AOctreeNode*, TInlineAllocator<6>> GetSurroundingBlocks(const FWorldPosition& Position);

	unsigned int GetRenderFaceMask(const FWorldPosition& Position);

	UPROPERTY(Category = "World|Chunk", EditDefaultsOnly)
	int32 SizeX;

	UPROPERTY(Category = "World|Chunk", EditDefaultsOnly)
	int32 SizeY;

	UPROPERTY(Category = "World|Chunk", EditDefaultsOnly)
	int32 SizeZ;

	UPROPERTY(Category = "World|Chunk", BlueprintReadOnly)
	FWorldPosition ChunkPosition;

private:

	//TODO: Implement octree
	AOctreeNode* ChunkBlocks[16][16][16];

};