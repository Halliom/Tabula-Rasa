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

	friend FORCEINLINE uint32 GetTypeHash(const FWorldPosition& Position)
	{
		return FCrc::MemCrc_DEPRECATED(&PositionX, sizeof(int32) * 3);
	}

	/**
	* Serialize to Archive
	*/
	friend FArchive& operator<<(FArchive& Ar, FWorldPosition& Position)
	{
		return Ar << Position.PositionX << Position.PositionY << Position.PositionZ;
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

	UPROPERTY(Category = "World|Chunk", BlueprintReadOnly)
	ASolidActor* NodeData;

	UPROPERTY(Category = "World|Chunk", BlueprintReadOnly)
	int Size;

	UPROPERTY(Category = "World|Chunk", BlueprintReadOnly)
	FVector Center;

	UPROPERTY(Category = "World|Chunk", BlueprintReadOnly)
	AChunk* Chunk;

private:

	AOctreeNode* RemoveChild(AOctreeNode* Node)
	{
		int32 Index = Children.Find(Node);
		AOctreeNode* Removed = Children[Index];
		Children.RemoveAt(Index, 1, false);

		return Removed;
	}

	AOctreeNode* ParentNode;

	TArray<AOctreeNode*, TInlineAllocator<8>> Children;

};

UCLASS()
class AChunk : public AActor
{
	GENERATED_BODY()
public:

	/**
	 * Constructor, sets off the BuildTree function to construct the tree
	 */
	AChunk(const FObjectInitializer& ObjectInitializer);

	/**
	 * Recursively dismounts the octree by calling its destructors
	 */
	~AChunk();

	//TODO: These need to be inlined or else...
	UFUNCTION(Category = "World|Chunk", BlueprintCallable)
	ASolidActor* GetNode(const FWorldPosition LocalTreePosition);

	UFUNCTION(Category = "World|Chunk", BlueprintCallable)
	void InsertIntoChunk(FWorldPosition LocalTreePosition, ASolidActor* Node);

	TArray<ASolidActor*, TInlineAllocator<6>> GetSurroundingBlocks(const FWorldPosition& Position);

	unsigned int GetRenderFaceMask(const FWorldPosition& Position);

	virtual void BeginPlay() override;

	UPROPERTY(Category = "World|Chunk", BlueprintReadOnly)
	FWorldPosition ChunkPosition;

private:

	/** 
	 * Builds the octree
	 */
	void BuildOctree(int Size);

private:

	/**
	 * The root node of the tree
	 */
	AOctreeNode* RootNode;

};