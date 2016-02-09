#include "VoxelLibPluginPrivatePCH.h"

#include "ChunkManager.h"
#include "VoxelMeshComponent.h"
#include "SolidActor.h"
#include "Chunk.h"

bool FWorldPosition::IsWithinBounds(const FWorldPosition& Bounds) const
{
	if ( (PositionX >= Bounds.PositionX) || (PositionX < 0) ||
		 (PositionY >= Bounds.PositionY) || (PositionY < 0) || 
		 (PositionZ >= Bounds.PositionZ) || (PositionZ < 0))
	{
		return false;
	}
	return true;
}

OctreeNode::OctreeNode() : 
	Size(0),
	NodeData(NULL),
	Location(0),
	Children(0)
{

}

OctreeNode::~OctreeNode()
{
	Children = 0;
	Location = 0;
	Size = 0;
	Children = 0;
	if (NodeData)
	{
		NodeData->Destroy();
		NodeData = NULL;
	}
}

AChunk::AChunk(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootNode = new OctreeNode();
	RootNode->Location = 1;
	RootNode->Size = 1024; //TODO: Don't have this hardcoded

	SizeX = SizeY = SizeZ = RootNode->Size;

	Tree.Add(1, RootNode);
}

AChunk::~AChunk()
{
	for (auto It = Tree.CreateConstIterator(); It; ++It)
	{
		OctreeNode* Node = NULL;
		Tree.RemoveAndCopyValue((*It).Key, Node);
		if (Node)
		{
			delete Node;
		}
	}
	Tree.Empty();
	RootNode = NULL;

	AChunkManager::GetStaticChunkManager()->DeleteChunkAtPosition(ChunkPosition);
}

OctreeNode * AChunk::GetNode(const FWorldPosition& Position, OctreeNode* Node)
{
	for (auto It = Tree.CreateConstIterator(); It; ++It)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pos: %d - (%d, %d, %d)"), (*It).Key, (*It).Value->Position.PositionX, (*It).Value->Position.PositionY, (*It).Value->Position.PositionZ);
	}
	UE_LOG(LogTemp, Warning, TEXT("---------"));

	if (!Position.IsWithinBounds(FWorldPosition(SizeX, SizeY, SizeZ)))
		return NULL;
	if (Node->Children != 0)
	{
		return GetNode(Position, Tree[(Node->Location << 3) | Node->GetOctantForPosition(Position)]);
	}
	else
	{
		if (Node->Position == Position)
		{
			return Node;
		}
	}
	return NULL;
}

void AChunk::RemoveNode(const FWorldPosition& Position, OctreeNode* Node)
{
	OctreeNode* NodeToBeDeleted = GetNode(Position, Node);
	if (NodeToBeDeleted)
	{
		if (NodeToBeDeleted->Children != 0)
		{
			// The node is now an empty node that just contains its children
			delete NodeToBeDeleted->NodeData;
		}
		else
		{
			Tree.Remove(NodeToBeDeleted->Location);
			// Check if all the sister nodes are empty - if so, remove the parent node aswell
			delete NodeToBeDeleted;
		}
	}
}

void AChunk::InsertNode(const FWorldPosition& Position, ASolidActor* NewVoxel, OctreeNode* Node)
{
	uint32 HalfSize = Node->Size >> 1;
	if (Node->Children == 0)
	{
		if (Node->NodeData == NULL || Node->Size == 1) //TODO: This should not be hardcoded
		{
			Node->NodeData = NewVoxel;
			Node->Position = Position;
			Node->NodeData->Chunk = this;
			Node->NodeData->OnNodePlacedAdjacent();
		}
		else
		{
			// There already exists something at this node
			// and it needs to get split
			ASolidActor* OldData = Node->NodeData;
			Node->NodeData = NULL;

			FWorldPosition OldPosition = Node->Position;

			for (uint32_t i = 0; i < 8; ++i)
			{
				OctreeNode* SplitNode = new OctreeNode();
				SplitNode->Location = (Node->Location << 3) | i;
				SplitNode->Size = HalfSize;
				Tree.Add(SplitNode->Location, SplitNode);
			}

			Node->Children = 255;

			InsertNode(OldPosition, OldData, Tree[(Node->Location << 3) | Node->GetOctantForPosition(OldPosition)]);
			InsertNode(Position, NewVoxel, Tree[(Node->Location << 3) | Node->GetOctantForPosition(Position)]);
		}
	}
	else
	{
		InsertNode(Position, NewVoxel, Tree[(Node->Location << 3) | Node->GetOctantForPosition(Position)]);
	}
}

FORCEINLINE unsigned int AChunk::GetRenderFaceMask(const FWorldPosition& Position)
{
	unsigned int Result = 63;

	ASolidActor* TopActor = GetNodeData(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ + 1));
	ASolidActor* BottomActor = GetNodeData(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ - 1));
	ASolidActor* FrontActor = GetNodeData(FWorldPosition(Position.PositionX, Position.PositionY + 1, Position.PositionZ));
	ASolidActor* BackActor = GetNodeData(FWorldPosition(Position.PositionX, Position.PositionY - 1, Position.PositionZ));
	ASolidActor* LeftActor = GetNodeData(FWorldPosition(Position.PositionX - 1, Position.PositionY, Position.PositionZ));
	ASolidActor* RightActor = GetNodeData(FWorldPosition(Position.PositionX + 1, Position.PositionY, Position.PositionZ));

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