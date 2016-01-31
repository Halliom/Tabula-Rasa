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
	GENERATED_BODY()
public:

	AOctreeNode(const FObjectInitializer& ObjectInitializer);

private: 

	AOctreeNode* ParentNode;

	TArray<AOctreeNode*, TInlineAllocator<8>> Children;

};

typedef TArray<AOctreeNode*, TInlineAllocator<8>> NodeDimension;

UCLASS()
class AChunk : public AActor
{
	GENERATED_BODY()
public:

	AChunk(const FObjectInitializer& ObjectInitializer);

	void InsertIntoChunkLocal(const FWorldPosition& LocalTreePosition, const AOctreeNode* node);

	UPROPERTY(Category = "World|Chunk", EditDefaultsOnly)
	int32 SizeX;

	UPROPERTY(Category = "World|Chunk", EditDefaultsOnly)
	int32 SizeY;

	UPROPERTY(Category = "World|Chunk", EditDefaultsOnly)
	int32 SizeZ;

private:

	//TODO: Implement octree
	TArray<TArray<TArray<AOctreeNode*, TInlineAllocator<16>>*, TInlineAllocator<16>>, TInlineAllocator<16>> ChunkBlocks;

};