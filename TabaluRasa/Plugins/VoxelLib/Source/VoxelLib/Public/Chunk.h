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

	FORCEINLINE bool IsWithinBounds(const FWorldPosition& Bounds) const
	{
		if ((this->PositionX < Bounds.PositionX) && (this->PositionX >= 0) &&
			(this->PositionY < Bounds.PositionY) && (this->PositionY >= 0) &&
			(this->PositionZ < Bounds.PositionZ) && (this->PositionZ >= 0))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	FORCEINLINE bool IsContainedWithin(const FWorldPosition& Min, const FWorldPosition& Max) const
	{
		if ((this->PositionX < Max.PositionX) && (this->PositionX >= Min.PositionX) &&
			(this->PositionY < Max.PositionY) && (this->PositionY >= Min.PositionY) &&
			(this->PositionZ < Max.PositionZ) && (this->PositionZ >= Min.PositionZ))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	FORCEINLINE bool operator==(const FWorldPosition& OtherPosition)
	{
		return PositionX == OtherPosition.PositionX && PositionY == OtherPosition.PositionY && PositionZ == OtherPosition.PositionZ;
	}

	FORCEINLINE void operator += (const int32& Operand)
	{
		this->PositionX += Operand;
		this->PositionY += Operand;
		this->PositionZ += Operand;
	}

	FORCEINLINE FWorldPosition operator+(const int32& Operand)
	{
		FWorldPosition Result = FWorldPosition(*this);
		Result.PositionX += Operand;
		Result.PositionY += Operand;
		Result.PositionZ += Operand;
		return Result;
	}

	FORCEINLINE FWorldPosition operator-(const int32& Operand)
	{
		FWorldPosition Result = FWorldPosition(*this);
		Result.PositionX -= Operand;
		Result.PositionY -= Operand;
		Result.PositionZ -= Operand;
		return Result;
	}
};

class ASolidActor;

//TODO: Should this be marked abstract?
class OctreeNode
{
public:

	OctreeNode();

	~OctreeNode();

	inline int GetOctantForPosition(const FWorldPosition& Position) const
	{
		int Result = 0;
		int32 HalfSize = Size >> 1;
		if (Position.PositionX >= Center.PositionX) // We are on the high X values
			Result |= 4;
		if (Position.PositionY >= Center.PositionY) // We are on the high Y values
			Result |= 2;
		if (Position.PositionZ >= Center.PositionZ)
			Result |= 1;
		return Result;
	}

	ASolidActor* NodeData;

	uint32 Location;

	uint32 Size;

	uint8 Children;

	FWorldPosition Position;

	FWorldPosition Center;

};

UCLASS(NotPlaceable)
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

	//OctreeNode* GetNodeAt(uint32_t Position);

	OctreeNode* GetNode(const FWorldPosition& Position, OctreeNode* Node);

	OctreeNode* GetNode(const FWorldPosition& Position)
	{
		return GetNode(Position, RootNode);
	}

	FORCEINLINE ASolidActor* GetNodeData(const FWorldPosition& Position)
	{
		OctreeNode* Node = GetNode(Position, RootNode);
		return Node ? Node->NodeData : NULL;
	}

	void RemoveNode(const FWorldPosition& Position, OctreeNode* Node, bool IsUpwardsRecursive = false);

	void RemoveNode(const FWorldPosition& Position)
	{
		RemoveNode(Position, RootNode);
	}

	void InsertNode(const FWorldPosition& Position, ASolidActor* NewVoxel, OctreeNode* Node, bool IsNew = true);

	//TODO: Should this be inlined?
	void InsertNode(const FWorldPosition& Position, ASolidActor* NewVoxel)
	{
		if (LastPlacedParentNode && Position.IsContainedWithin(LastPlacedParentNodeBoxMin, LastPlacedParentNodeBoxMax))
		{
			InsertNode(Position, NewVoxel, LastPlacedParentNode);
		}
		else
		{
			InsertNode(Position, NewVoxel, RootNode);
		}
	}

	void FindAdjacentNodes(const FWorldPosition& Position, OctreeNode* Node, TArray<ASolidActor*>& OutArray);

	unsigned int GetRenderFaceMask(const FWorldPosition& Position);

	UPROPERTY(Category = "World|Chunk", BlueprintReadOnly)
	FWorldPosition ChunkPosition;

private:

	/**
	 * The root node of the tree
	 */
	OctreeNode* RootNode;

	FWorldPosition LastPlacedParentNodeBoxMin;
	FWorldPosition LastPlacedParentNodeBoxMax;

	OctreeNode* LastPlacedParentNode;

	TMap<uint32, OctreeNode*> Tree;

	FWorldPosition Extent;
};