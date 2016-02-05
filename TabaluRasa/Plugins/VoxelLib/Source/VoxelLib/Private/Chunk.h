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

	bool operator==(const FWorldPosition& OtherPosition)
	{
		return PositionX == OtherPosition.PositionX && PositionY == OtherPosition.PositionY && PositionZ == OtherPosition.PositionZ;
	}
};

class ASolidActor;

//TODO: Should this be marked abstract?
class AOctreeNode
{
	friend class AChunk;
public:

	AOctreeNode();

	AOctreeNode(AOctreeNode* Parent, FVector Center, AChunk* Chunk) : 
		ParentNode(Parent),
		Center(Center),
		Chunk(Chunk)
	{}

	void InsertNode(const FWorldPosition& InsertPosition, ASolidActor* NodeData);

	AOctreeNode* GetNodeAtPosition(const FWorldPosition& Position) const;

	AOctreeNode* RemoveNodeAtPosition(const FWorldPosition& Position);

	inline int GetOctantForPosition(const FWorldPosition& Position) const
	{
		int Result = 0;
		if (Position.PositionX >= Center.X) // We are on the high X values
			Result |= 4;
		if (Position.PositionY >= Center.Y) // We are on the high Y values
			Result |= 2;
		if (Position.PositionZ >= Center.Z)
			Result |= 1;
		return Result;
	}

	UPROPERTY(Category = "Chunk", BlueprintReadOnly)
	ASolidActor* NodeData;

	AOctreeNode* RemoveChild(AOctreeNode* Node)
	{
		int32 Index = Children.Find(Node);
		AOctreeNode* Removed = Children[Index];
		Children.RemoveAt(Index, 1, false);

		return Removed;
	}

	AOctreeNode* ParentNode;

	TArray<AOctreeNode*, TInlineAllocator<8>> Children;

	//TODO: Do we really need a box?
	//UPROPERTY()
	//FBox Box;

	int Size;

	UPROPERTY()
	FVector Center;

	UPROPERTY()
	AChunk* Chunk;

};

UCLASS()
class AChunk : public AActor
{
	GENERATED_BODY()
public:

	AChunk(const FObjectInitializer& ObjectInitializer);

	~AChunk();

	AOctreeNode* GetNodeFromTreeLocal(const FWorldPosition LocalTreePosition);

	UFUNCTION(Category = "Chunk", BlueprintCallable)
	void InsertIntoChunkLocal(FWorldPosition LocalTreePosition, ASolidActor* Node);

	TArray<AOctreeNode*, TInlineAllocator<6>> GetSurroundingBlocks(const FWorldPosition& Position);

	unsigned int GetRenderFaceMask(const FWorldPosition& Position);

	virtual void BeginPlay() override;

	UPROPERTY(Category = "World|Chunk", BlueprintReadOnly)
	FWorldPosition ChunkPosition;

private:

	void BuildOctree(int Size);

private:

	AOctreeNode* RootNode;

};