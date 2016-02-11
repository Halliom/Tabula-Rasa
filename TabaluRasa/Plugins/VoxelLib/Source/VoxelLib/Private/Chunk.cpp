#include "VoxelLibPluginPrivatePCH.h"

#include "ChunkManager.h"
#include "VoxelMeshComponent.h"
#include "SolidActor.h"
#include "Chunk.h"

bool FWorldPosition::IsWithinBounds(const FWorldPosition& Bounds) const
{
	if ( (this->PositionX < Bounds.PositionX) && (this->PositionX >= 0) &&
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

	uint32_t HalfSize = RootNode->Size >> 1;
	RootNode->Center = FWorldPosition(HalfSize, HalfSize, HalfSize);

	Extent = FWorldPosition(RootNode->Size, RootNode->Size, RootNode->Size);

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
	if (!Position.IsWithinBounds(Extent))
	{
		return NULL;
	}
	else
	{
		if (Node->Children != 0 && Node->Size != 1)
		{
			uint32 Octant = Node->GetOctantForPosition(Position);

			if ((Node->Children >> Octant) & 1)
			{
				return GetNode(Position, Tree[(Node->Location << 3) | Octant]);
			}
		}
		else
		{
			if (Node->Position == Position)
			{
				return Node;
			}
		}
	}
	return NULL;
}

void AChunk::RemoveNode(const FWorldPosition& Position, OctreeNode* Node)
{
	//TODO: Needs complete rewrite to sparse voxel octree
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

void AChunk::InsertNode(const FWorldPosition& Position, ASolidActor* NewVoxel, OctreeNode* Node, bool IsNew)
{
	uint32 HalfSize = Node->Size >> 1;
	uint32 QuarterSize = HalfSize >> 1;
	if (Node->Children == 0)
	{
		if (Node->NodeData == NULL || Node->Size == 1)
		{
			Node->NodeData = NewVoxel;
			Node->Position = Position;

			// Only do these if it is a new node, not a node being split
			if (IsNew && !Node->NodeData->Chunk)
			{
				Node->NodeData->Chunk = this;
				Node->NodeData->LocalChunkPosition = Position;
				Node->NodeData->OnNodePlacedAdjacent();
			}

			if (!Node->Location == 1) // The RootNode doesnt have a parent
			{
				// Set the children of the parent to this octant
				uint32 Octant = Node->Location & 7; //0b111
				Tree[(Node->Location >> 3)]->Children |= (1 << Octant);
			}
		}
		else
		{
			// There already exists something at this node
			// and it needs to get split
			ASolidActor* OldData = Node->NodeData;
			Node->NodeData = NULL;

			FWorldPosition OldPosition = Node->Position;
			uint32 OldNodeOctant = Node->GetOctantForPosition(OldPosition);
			uint32 NewNodeOctant = Node->GetOctantForPosition(Position);

			if (OldNodeOctant == NewNodeOctant)
			{
				// Create one child node for this node and let the recursion handle
				// the rest of the splitting
				OctreeNode* OldNode = new OctreeNode();
				OldNode->Location = (Node->Location << 3) | OldNodeOctant;
				OldNode->Size = HalfSize;
				OldNode->Center.PositionX = Node->Center.PositionX + ((int32) QuarterSize) * ((OldNodeOctant & 4) ? 1 : -1);
				OldNode->Center.PositionY = Node->Center.PositionY + ((int32) QuarterSize) * ((OldNodeOctant & 2) ? 1 : -1);
				OldNode->Center.PositionZ = Node->Center.PositionZ + ((int32) QuarterSize) * ((OldNodeOctant & 1) ? 1 : -1);
				Tree.Add(OldNode->Location, OldNode);

				Node->Children |= (1 << OldNodeOctant);

				InsertNode(OldPosition, OldData, OldNode, false);
				InsertNode(Position, NewVoxel, OldNode, true);
			}
			else
			{
				OctreeNode* OldNode = new OctreeNode();
				OldNode->Location = (Node->Location << 3) | OldNodeOctant;
				OldNode->Size = HalfSize;
				OldNode->Center.PositionX = Node->Center.PositionX + ((int32) QuarterSize) * ((OldNodeOctant & 4) ? 1 : -1);
				OldNode->Center.PositionY = Node->Center.PositionY + ((int32) QuarterSize) * ((OldNodeOctant & 2) ? 1 : -1);
				OldNode->Center.PositionZ = Node->Center.PositionZ + ((int32) QuarterSize) * ((OldNodeOctant & 1) ? 1 : -1);
				Tree.Add(OldNode->Location, OldNode);

				OctreeNode* NewNode = new OctreeNode();
				NewNode->Location = (Node->Location << 3) | NewNodeOctant;
				NewNode->Size = HalfSize;
				NewNode->Center.PositionX = Node->Center.PositionX + ((int32) QuarterSize) * ((NewNodeOctant & 4) ? 1 : -1);
				NewNode->Center.PositionY = Node->Center.PositionY + ((int32) QuarterSize) * ((NewNodeOctant & 2) ? 1 : -1);
				NewNode->Center.PositionZ = Node->Center.PositionZ + ((int32) QuarterSize) * ((NewNodeOctant & 1) ? 1 : -1);
				Tree.Add(NewNode->Location, NewNode);

				Node->Children |= (1 << OldNodeOctant);
				Node->Children |= (1 << NewNodeOctant);

				InsertNode(OldPosition, OldData, OldNode, false);
				InsertNode(Position, NewVoxel, NewNode, true);
			}
		}
	}
	else
	{
		uint32 NodeOctant = Node->GetOctantForPosition(Position);

		if ((Node->Children >> NodeOctant) & 1) // It has a child in that octant
		{
			InsertNode(Position, NewVoxel, Tree[(Node->Location << 3) | NodeOctant]);
		}
		else // It doesn't have a node in that octant, create it
		{
			OctreeNode* NewNode = new OctreeNode();
			NewNode->Location = (Node->Location << 3) | NodeOctant;
			NewNode->Size = HalfSize;
			NewNode->Center.PositionX = Node->Center.PositionX + ((int32) QuarterSize) * ((NodeOctant & 4) ? 1 : -1);
			NewNode->Center.PositionY = Node->Center.PositionY + ((int32) QuarterSize) * ((NodeOctant & 2) ? 1 : -1);
			NewNode->Center.PositionZ = Node->Center.PositionZ + ((int32) QuarterSize) * ((NodeOctant & 1) ? 1 : -1);
			Node->Children |= (1 << NodeOctant);
			Tree.Add(NewNode->Location, NewNode);

			InsertNode(Position, NewVoxel, NewNode, true);
		}
	}
}

FORCEINLINE unsigned int AChunk::GetRenderFaceMask(const FWorldPosition& Position)
{
	unsigned int Result = 63;

	for (auto It = Tree.CreateConstIterator(); It; ++It)
	{
		if ((*It).Value->NodeData)
		{
			UE_LOG(LogTemp, Warning, TEXT("%d - (%d, %d, %d)"), (*It).Key, (*It).Value->Position.PositionX, (*It).Value->Position.PositionY, (*It).Value->Position.PositionZ);
		}
	}

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