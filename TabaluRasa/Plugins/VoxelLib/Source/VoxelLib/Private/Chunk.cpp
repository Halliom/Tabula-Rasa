#include "VoxelLibPluginPrivatePCH.h"

#include "VoxelMeshComponent.h"
#include "SolidActor.h"
#include "Chunk.h"

bool FWorldPosition::IsWithinBounds(const FWorldPosition& Bounds) const
{
	if ((PositionX < Bounds.PositionX) && (PositionX >= 0))
	{
		if ((PositionY < Bounds.PositionY) && (PositionY >= 0))
		{
			if ((PositionZ < Bounds.PositionZ) && (PositionZ >= 0))
			{
				return true;
			}
		}
	}
	return false;
}

AOctreeNode::AOctreeNode()
{
}

void AOctreeNode::InsertNode(const FWorldPosition& InsertPosition, ASolidActor* NodeData)
{
	int HalfSize = Size >> 1;

	// This tests whether we are a "leaf node" or not. A leaf node doesn't contain
	// any children and since all the children get initialized if we have children
	// then all of the elements will be NULL if we don't have ANY children and
	// therefore this check is sufficient.
	if (Children[0] == NULL)
	{
		// No worries this node is empty, just set the data. Or this is as far down
		// the tree as we will go and then we will have to replace the
		if (NodeData == NULL || HalfSize == 0)
		{
			this->NodeData = NodeData;
			this->NodeData->LocalChunkPosition = InsertPosition;
			this->NodeData->ContainingNode = this;

			this->NodeData->OnNodePlacedAdjacent();
		}
		else
		{
			// There already is something on this node so it has to get broken up
			// and then we have to set the old NodeData in the correct octant and
			// then the new NodeData in its correct octant.

			ASolidActor* OldNodeData = this->NodeData;
			this->NodeData = NULL;

			for (int i = 0; i < 8; ++i)
			{
				// Bitmasks out the x coordinate by checking the 3rd bit, the y
				// by checking the 2nd and the z by checking the 1st bit. Then
				// multiplies the x, y and z values for the new center to be either
				// to the right of / up down / in out of the current nodes center
				FVector NewCenter = FVector(Center);
				NewCenter.X += HalfSize * (i & 4 ? 0.5f : -0.5f);
				NewCenter.Y += HalfSize * (i & 2 ? 0.5f : -0.5f);
				NewCenter.Z += HalfSize * (i & 1 ? 0.5f : -0.5f);
				Children[i] = new AOctreeNode(this, NewCenter, Chunk);
			}

			// Insert the new node
			Children[GetOctantForPosition(InsertPosition)]->InsertNode(InsertPosition, NodeData);

			// (Re)Insert the old node
			Children[GetOctantForPosition(OldNodeData->LocalChunkPosition)]->InsertNode(OldNodeData->LocalChunkPosition, OldNodeData);
		}
	}
	else
	{
		// This node is subdivided and we just need to get the correct region to put
		// the NodeData in.
		Children[GetOctantForPosition(InsertPosition)]->InsertNode(InsertPosition, NodeData);
	}
}

FORCEINLINE AOctreeNode* AOctreeNode::GetNodeAtPosition(const FWorldPosition& Position) const
{
	if (Children[0] == NULL)
	{
		if (NodeData && (NodeData->LocalChunkPosition == Position))
		{
			return (AOctreeNode*) this;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return Children[GetOctantForPosition(Position)]->GetNodeAtPosition(Position);
	}
}

FORCEINLINE AOctreeNode* AOctreeNode::RemoveNodeAtPosition(const FWorldPosition& Position)
{
	if (Children[0] == NULL)
	{
		if (NodeData && (NodeData->LocalChunkPosition == Position))
		{
			if (ParentNode)
			{
				return ParentNode->RemoveChild(this);
			}
		}
		return NULL;
	}
	else
	{
		return Children[GetOctantForPosition(Position)]->RemoveNodeAtPosition(Position);
	}
}

AChunk::AChunk(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BuildOctree(INITIAL_CHUNK_SIZE);
}

AChunk::~AChunk()
{
	if (RootNode)
	{
		delete RootNode;
	}
}

void AChunk::BuildOctree(int Size)
{
	RootNode = new AOctreeNode();

	int HalfSize = Size >> 1;
	RootNode->Center = FVector(HalfSize, HalfSize, HalfSize);
	//TODO: Set the world position of the node

	RootNode->Chunk = this;
	RootNode->Size = Size;
}

void AChunk::BeginPlay()
{
}

ASolidActor* AChunk::GetNode(const FWorldPosition LocalTreePosition)
{
	AOctreeNode* Node = RootNode->GetNodeAtPosition(LocalTreePosition);
	return Node ? Node->NodeData : NULL;
}

void AChunk::InsertIntoChunk(FWorldPosition LocalTreePosition, ASolidActor* Node)
{
	RootNode->InsertNode(LocalTreePosition, Node);
}

FORCEINLINE TArray<ASolidActor*, TInlineAllocator<6>> AChunk::GetSurroundingBlocks(const FWorldPosition& Position)
{
	TArray<ASolidActor*, TInlineAllocator<6>> Result;

	Result.Add(GetNode(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ + 1)));
	Result.Add(GetNode(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ - 1)));
	Result.Add(GetNode(FWorldPosition(Position.PositionX, Position.PositionY + 1, Position.PositionZ)));
	Result.Add(GetNode(FWorldPosition(Position.PositionX, Position.PositionY - 1, Position.PositionZ)));
	Result.Add(GetNode(FWorldPosition(Position.PositionX - 1, Position.PositionY, Position.PositionZ)));
	Result.Add(GetNode(FWorldPosition(Position.PositionX + 1, Position.PositionY, Position.PositionZ)));

	return Result;
}

unsigned int AChunk::GetRenderFaceMask(const FWorldPosition& Position)
{
	unsigned int Result;

	if (!GetNode(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ + 1)))
	{
		Result |= EVoxelSide::VS_SIDE_TOP;
	}
	if (!GetNode(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ - 1)))
	{
		Result |= EVoxelSide::VS_SIDE_BOTTOM;
	}
	if (!GetNode(FWorldPosition(Position.PositionX, Position.PositionY + 1, Position.PositionZ)))
	{
		Result |= EVoxelSide::VS_SIDE_FRONT;
	}
	if (!GetNode(FWorldPosition(Position.PositionX, Position.PositionY - 1, Position.PositionZ)))
	{ 
		Result |= EVoxelSide::VS_SIDE_BACK;
	}
	if (!GetNode(FWorldPosition(Position.PositionX - 1, Position.PositionY, Position.PositionZ)))
	{
		Result |= EVoxelSide::VS_SIDE_LEFT;
	}
	if (!GetNode(FWorldPosition(Position.PositionX + 1, Position.PositionY, Position.PositionZ)))
	{
		Result |= EVoxelSide::VS_SIDE_RIGHT;
	}

	return Result;
}