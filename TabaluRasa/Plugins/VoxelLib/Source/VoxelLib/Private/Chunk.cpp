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

void AOctreeNode::InsertNode(const FWorldPosition& LocalPosition)
{

}

AChunk::AChunk(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//RootNode = ObjectInitializer.CreateDefaultSubobject<AOctreeNode>(this, TEXT("RootNode"));
	//RootComponent = RootNode;
}

AOctreeNode* AChunk::GetNodeFromTreeLocal(const FWorldPosition& LocalTreePosition)
{
	if (LocalTreePosition.IsWithinBounds(FWorldPosition(16, 16, 16)))
	{
		return ChunkBlocks[LocalTreePosition.PositionX][LocalTreePosition.PositionY][LocalTreePosition.PositionZ];
	}
	return NULL;
}

void AChunk::InsertIntoChunkLocal(const FWorldPosition& LocalTreePosition, AOctreeNode* Node)
{
	if (LocalTreePosition.IsWithinBounds(FWorldPosition(16, 16, 16)))
	{
		ChunkBlocks[LocalTreePosition.PositionX][LocalTreePosition.PositionY][LocalTreePosition.PositionZ] = Node;
		Node->Chunk = this;
		Node->LocalPosition = LocalTreePosition;
		//TODO: Set World position
	}
}

TArray<AOctreeNode*, TInlineAllocator<6>> AChunk::GetSurroundingBlocks(const FWorldPosition& Position)
{
	TArray<AOctreeNode*, TInlineAllocator<6>> Result;

	Result[0] = GetNodeFromTreeLocal(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ + 1));
	Result[1] = GetNodeFromTreeLocal(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ - 1));
	Result[2] = GetNodeFromTreeLocal(FWorldPosition(Position.PositionX, Position.PositionY + 1, Position.PositionZ));
	Result[3] = GetNodeFromTreeLocal(FWorldPosition(Position.PositionX, Position.PositionY - 1, Position.PositionZ));
	Result[4] = GetNodeFromTreeLocal(FWorldPosition(Position.PositionX - 1, Position.PositionY, Position.PositionZ));
	Result[5] = GetNodeFromTreeLocal(FWorldPosition(Position.PositionX + 1, Position.PositionY, Position.PositionZ));

	return Result;
}