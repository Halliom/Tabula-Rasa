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

OctreeNode::OctreeNode()
{
	Children.SetNumZeroed(8);
	Size = 0;
	Center = FVector::ZeroVector;
	Chunk = NULL;
	NodeData = NULL;
}

OctreeNode::~OctreeNode()
{
	if (NodeData)
	{
		NodeData->Destroy();
		NodeData = NULL;
	}
	if (Children[0] == NULL)
	{
		for (auto Child : Children)
		{
			delete Child;
		}
	}
	Chunk = NULL;
	ParentNode = NULL;
}

void OctreeNode::InsertNode(const FWorldPosition& InsertPosition, ASolidActor* NewNodeData)
{
	if (NewNodeData == NULL)
		return;

	int HalfSize = Size >> 1;

	// This tests whether we are a "leaf node" or not. A leaf node doesn't contain
	// any children and since all the children get initialized if we have children
	// then all of the elements will be NULL if we don't have ANY children and
	// therefore this check is sufficient.
	if (Children[0] == NULL)
	{
		// No worries this node is empty, just set the data. Or this is as far down
		// the tree as we will go and then we will have to replace the
		if (this->NodeData == NULL || HalfSize == 0)
		{
			this->NodeData = NewNodeData;
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
				Children[i] = new OctreeNode(this, NewCenter, Chunk, HalfSize);
			}

			// Insert the new node
			Children[GetOctantForPosition(InsertPosition)]->InsertNode(InsertPosition, NewNodeData);

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

FORCEINLINE OctreeNode* OctreeNode::GetNodeAtPosition(const FWorldPosition& Position) const
{
	if (Children[0] == NULL)
	{
		if (NodeData)
		{
			if ((NodeData->LocalChunkPosition == Position))
			{
				return (OctreeNode*) this;
			}
		}
		return NULL;
	}
	else
	{
		return Children[GetOctantForPosition(Position)]->GetNodeAtPosition(Position);
	}
}

FORCEINLINE OctreeNode* OctreeNode::RemoveNodeAtPosition(const FWorldPosition& Position)
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
	int HalfSize = Size >> 1;
	RootNode = new OctreeNode(NULL, FVector(HalfSize, HalfSize, HalfSize), this, Size);
}

void AChunk::BeginPlay()
{
	Super::BeginPlay();
}

ASolidActor* AChunk::GetNode(const FWorldPosition LocalTreePosition)
{
	OctreeNode* Node = RootNode->GetNodeAtPosition(LocalTreePosition);
	return Node ? Node->NodeData : NULL;
}

void AChunk::InsertIntoChunk(FWorldPosition LocalTreePosition, ASolidActor* Node)
{
	if (RootNode)
	{
		RootNode->InsertNode(LocalTreePosition, Node);
	}
	else
	{
		BuildOctree(INITIAL_CHUNK_SIZE);
		RootNode->InsertNode(LocalTreePosition, Node);
	}
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

FORCEINLINE unsigned int AChunk::GetRenderFaceMask(const FWorldPosition& Position)
{
	unsigned int Result = 63;

	ASolidActor* TopActor = GetNode(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ + 1));
	ASolidActor* BottomActor = GetNode(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ - 1));
	ASolidActor* FrontActor = GetNode(FWorldPosition(Position.PositionX, Position.PositionY + 1, Position.PositionZ));
	ASolidActor* BackActor = GetNode(FWorldPosition(Position.PositionX, Position.PositionY - 1, Position.PositionZ));
	ASolidActor* LeftActor = GetNode(FWorldPosition(Position.PositionX - 1, Position.PositionY, Position.PositionZ));
	ASolidActor* RightActor = GetNode(FWorldPosition(Position.PositionX + 1, Position.PositionY, Position.PositionZ));

	if (TopActor)
	{
		Result ^= EVoxelSide::VS_SIDE_TOP;
		TopActor->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_BOTTOM);
	}
	if (BottomActor)
	{
		Result ^= EVoxelSide::VS_SIDE_BOTTOM;
		BottomActor->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_TOP);
	}
	if (FrontActor)
	{
		Result ^= EVoxelSide::VS_SIDE_FRONT;
		FrontActor->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_BACK);
	}
	if (BackActor)
	{
		Result ^= EVoxelSide::VS_SIDE_BACK;
		BackActor->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_FRONT);
	}
	if (LeftActor)
	{
		Result ^= EVoxelSide::VS_SIDE_LEFT;
		LeftActor->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_RIGHT);
	}
	if (RightActor)
	{
		Result ^= EVoxelSide::VS_SIDE_RIGHT;
		RightActor->OnNodePlacedOnSide(EVoxelSide::VS_SIDE_LEFT);
	}

	return Result;
}