#pragma once

#include "VoxelLibPluginPrivatePCH.h"

#include "Chunk.generated.h"

#define INITIAL_CHUNK_SIZE 64

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
class OctreeNode
{
	friend class AChunk;
public:

	OctreeNode();

	OctreeNode(OctreeNode* Parent, FVector Center, AChunk* Chunk, int Size) : 
		ParentNode(Parent),
		Center(Center),
		Chunk(Chunk),
		Size(Size),
		NodeData(NULL)
	{
		Children.SetNumZeroed(8);
	}

	~OctreeNode();

	void InsertNode(const FWorldPosition& InsertPosition, ASolidActor* NodeData);

	OctreeNode* GetNodeAtPosition(const FWorldPosition& Position) const;

	OctreeNode* RemoveNodeAtPosition(const FWorldPosition& Position);

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

	ASolidActor* NodeData;

	int Size;

	FVector Center;

	AChunk* Chunk;

private:

	OctreeNode* RemoveChild(OctreeNode* Node)
	{
		int32 Index = Children.Find(Node);
		OctreeNode* Removed = Children[Index];
		Children.RemoveAt(Index, 1, false);

		return Removed;
	}

	OctreeNode* ParentNode;

	TArray<OctreeNode*, TInlineAllocator<8>> Children;

};

UCLASS()
class VOXELLIB_API AChunk : public AActor
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
	OctreeNode* RootNode;
};