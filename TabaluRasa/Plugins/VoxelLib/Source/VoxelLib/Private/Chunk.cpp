#include "VoxelLibPluginPrivatePCH.h"

#include "Chunk.h"

bool FWorldPosition::IsWithinBounds(const FWorldPosition& Bounds) const
{
	if ((PositionX <= Bounds.PositionX) && (PositionY <= Bounds.PositionY) && (PositionZ <= Bounds.PositionZ))
	{
		return true;
	}
	else
	{
		return false;
	}
}

AOctreeNode::AOctreeNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

AChunk::AChunk(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//RootNode = ObjectInitializer.CreateDefaultSubobject<AOctreeNode>(this, TEXT("RootNode"));
}

void AChunk::InsertIntoChunkLocal(const FWorldPosition& LocalTreePosition, const AOctreeNode* node)
{
	if (LocalTreePosition.IsWithinBounds(FWorldPosition(16, 16, 16)))
	{

	}
}