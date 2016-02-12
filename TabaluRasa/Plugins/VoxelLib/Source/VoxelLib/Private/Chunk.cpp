#include "VoxelLibPluginPrivatePCH.h"

#include "ChunkManager.h"
#include "VoxelMeshComponent.h"
#include "SolidActor.h"
#include "Chunk.h"

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

FORCEINLINE OctreeNode * AChunk::GetNode(const FWorldPosition& Position, OctreeNode* Node)
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

void AChunk::RemoveNode(const FWorldPosition& Position, OctreeNode* Node, bool IsUpwardsRecursive)
{
	//TODO: Needs complete rewrite to sparse voxel octree
	OctreeNode* NodeToBeDeleted = GetNode(Position, Node);
	if (NodeToBeDeleted)
	{
		if (NodeToBeDeleted->Children != 0 && !IsUpwardsRecursive)
		{
			// Here we need to visit every child and add them to a list to then re-add them
			// to the tree once this node is gone
		}
		else if (Node->Location != 1)
		{
			uint32 NodeOctant = Node->Location & 7;
			OctreeNode* ParentNode = Tree[(Node->Location >> 3)];
			ParentNode->Children ^= (1 << NodeOctant); // Remove from parents children
			RemoveNode(ParentNode->Position, ParentNode, true);

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

				if (Node->Location != 1)
				{
					TArray<ASolidActor*> AdjacentNodes;
					AdjacentNodes.SetNum(6);
					FindAdjacentNodes(Position, Tree[(Node->Location >> 3)], AdjacentNodes);

					Node->NodeData->OnNodePlacedAdjacent(AdjacentNodes);
				}
			}

			if (Node->Location != 1) // The RootNode doesnt have a parent
			{
				OctreeNode* ParentNode = Tree[(Node->Location >> 3)];

				// Set the children of the parent to this octant
				uint32 Octant = Node->Location & 7; //0b111
				ParentNode->Children |= (1 << Octant);

				LastPlacedParentNode = ParentNode;
				LastPlacedParentNodeBoxMin = ParentNode->Center - Node->Size;
				LastPlacedParentNodeBoxMax = ParentNode->Center + Node->Size;
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
				OldNode->Center = Node->Center;
				OldNode->Center.PositionX += ((int32) QuarterSize) * ((OldNodeOctant & 4) ? 1 : -1);
				OldNode->Center.PositionY += ((int32) QuarterSize) * ((OldNodeOctant & 2) ? 1 : -1);
				OldNode->Center.PositionZ += ((int32) QuarterSize) * ((OldNodeOctant & 1) ? 1 : -1);
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
				OldNode->Center = Node->Center;
				OldNode->Center.PositionX += ((int32) QuarterSize) * ((OldNodeOctant & 4) ? 1 : -1);
				OldNode->Center.PositionY += ((int32) QuarterSize) * ((OldNodeOctant & 2) ? 1 : -1);
				OldNode->Center.PositionZ += ((int32) QuarterSize) * ((OldNodeOctant & 1) ? 1 : -1);
				Tree.Add(OldNode->Location, OldNode);

				OctreeNode* NewNode = new OctreeNode();
				NewNode->Location = (Node->Location << 3) | NewNodeOctant;
				NewNode->Size = HalfSize;
				NewNode->Center = Node->Center;
				NewNode->Center.PositionX += ((int32) QuarterSize) * ((NewNodeOctant & 4) ? 1 : -1);
				NewNode->Center.PositionY += ((int32) QuarterSize) * ((NewNodeOctant & 2) ? 1 : -1);
				NewNode->Center.PositionZ += ((int32) QuarterSize) * ((NewNodeOctant & 1) ? 1 : -1);
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
			NewNode->Center = Node->Center;
			NewNode->Center.PositionX += ((int32) QuarterSize) * ((NodeOctant & 4) ? 1 : -1);
			NewNode->Center.PositionY += ((int32) QuarterSize) * ((NodeOctant & 2) ? 1 : -1);
			NewNode->Center.PositionZ += ((int32) QuarterSize) * ((NodeOctant & 1) ? 1 : -1);
			Node->Children |= (1 << NodeOctant);
			Tree.Add(NewNode->Location, NewNode);

			InsertNode(Position, NewVoxel, NewNode, true);
		}
	}
}

FORCEINLINE void AChunk::FindAdjacentNodes(const FWorldPosition& Position, OctreeNode* ParentNode, TArray<ASolidActor*>& OutArray)
{
	OutArray[0] = GetNodeData(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ + 1));
	OutArray[1] = GetNodeData(FWorldPosition(Position.PositionX, Position.PositionY, Position.PositionZ - 1));
	OutArray[2] = GetNodeData(FWorldPosition(Position.PositionX, Position.PositionY + 1, Position.PositionZ));
	OutArray[3] = GetNodeData(FWorldPosition(Position.PositionX, Position.PositionY - 1, Position.PositionZ));
	OutArray[4] = GetNodeData(FWorldPosition(Position.PositionX - 1, Position.PositionY, Position.PositionZ));
	OutArray[5] = GetNodeData(FWorldPosition(Position.PositionX + 1, Position.PositionY, Position.PositionZ));
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