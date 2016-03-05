#include "Octree.h"

#include <intrin.h>

glm::uvec3 Chunk::VS_EAST_OFFSET = glm::uvec3(1, 0, 0);
glm::uvec3 Chunk::VS_WEST_OFFSET = glm::uvec3(-1, 0, 0);
glm::uvec3 Chunk::VS_TOP_OFFSET = glm::uvec3(0, 1, 0);
glm::uvec3 Chunk::VS_BOTTOM_OFFSET = glm::uvec3(0, -1, 0);
glm::uvec3 Chunk::VS_NORTH_OFFSET = glm::uvec3(0, 0, 1);
glm::uvec3 Chunk::VS_SOUTH_OFFSET = glm::uvec3(0, 0, -1);

void Voxel::OnNodeUpdatedAdjacent(const uint8_t& X, const uint8_t& Y, const uint8_t& Z, Voxel* NodeEast, Voxel* NodeWest, Voxel* NodeTop, Voxel* NodeBottom, Voxel* NodeNorth, Voxel* NodeSouth, const bool& Placed)
{
	uint8_t Result = 63;
	ChunkRenderCoordinate Coord = ChunkRenderCoordinate(X, Y, Z);
	if (NodeEast)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_EAST;
		NodeEast->OnNodeUpdatedOnSide(X + 1, Y, Z, VoxelSide::SIDE_WEST, Placed);
	}
	if (NodeWest)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_WEST;
		NodeWest->OnNodeUpdatedOnSide(X - 1, Y, Z, VoxelSide::SIDE_EAST, Placed);
	}
	if (NodeTop)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_TOP;
		NodeTop->OnNodeUpdatedOnSide(X, Y + 1, Z, VoxelSide::SIDE_BOTTOM, Placed);
	}
	if (NodeBottom)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_BOTTOM;
		NodeBottom->OnNodeUpdatedOnSide(X, Y - 1, Z, VoxelSide::SIDE_TOP, Placed);
	}
	if (NodeNorth)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_NORTH;
		NodeNorth->OnNodeUpdatedOnSide(X, Y, Z + 1, VoxelSide::SIDE_SOUTH, Placed);
	}
	if (NodeSouth)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_SOUTH;
		NodeSouth->OnNodeUpdatedOnSide(X, Y, Z - 1, VoxelSide::SIDE_NORTH, Placed);
	}

	if (!Placed)
	{
		SidesToRender = 0;
	}
	else
	{
		SidesToRender = Result;
	}
}

// Placed means that the original block which called this function was placed not removed
__forceinline void Voxel::OnNodeUpdatedOnSide(const uint8_t& X, const uint8_t& Y, const uint8_t& Z, const VoxelSide& Side, const bool& Placed)
{
	if (Placed)
	{
		// Make sure it does NOT render
		SidesToRender ^= Side;
	}
	else
	{
		// Make sure it DOES render
		SidesToRender |= Side;
	}
}

__forceinline bool IsWithinBounds(const glm::uvec3& Position, const glm::uvec3& Bounds) //TODO: SIMD optimize from custom vector class
{
	if ((Position.x < Bounds.x) && (Position.x >= 0) &&
		(Position.y < Bounds.y) && (Position.y >= 0) &&
		(Position.z < Bounds.z) && (Position.z >= 0))
		return true;
	else
		return false;
}

__forceinline bool IsContainedWithin(const glm::uvec3& Position, const glm::uvec3& Min, const glm::uvec3& Max) //TODO: SIMD optimize from custom vector class
{
	if ((Position.x < Max.x) && (Position.x >= Min.x) &&
		(Position.y < Max.y) && (Position.y >= Min.y) &&
		(Position.z < Max.z) && (Position.z >= Min.z))
		return true;
	else
		return false;
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
	if (NodeData)
	{
		delete NodeData;
		NodeData = NULL;
	}
}

size_t OctreeNode::GetNodeDepth()
{
	unsigned long msb;
	_BitScanReverse(&msb, this->Location);
	return msb / 3;
}

__forceinline size_t OctreeNode::GetOctantForPosition(const glm::uvec3& Position)
{
	size_t Result = 0;
	uint32_t HalfSize = Size >> 1;
	if (Position.x >= Center.x) Result |= 4;
	if (Position.y >= Center.y) Result |= 2;
	if (Position.z >= Center.z) Result |= 1;
	return Result;
}

void Chunk::InsertNode(const glm::uvec3& Position, Voxel* NewVoxel, OctreeNode* Node, bool IsNew)
{
	uint32_t HalfSize = Node->Size >> 1;
	uint32_t QuarterSize = HalfSize >> 1;
	if (Node->Children == 0)
	{
		if (Node->NodeData == NULL || Node->Size == 1)
		{
			Node->NodeData = NewVoxel;
			Node->ChunkPosition = Position;

			if (IsNew && !Node->NodeData->Chunk)
			{
				Node->NodeData->Chunk = this;
				//Node->NodeData->LocalChunkPosition = Position;

				Node->NodeData->OnNodeUpdatedAdjacent(
					(uint8_t) Position.x,
					(uint8_t) Position.y,
					(uint8_t) Position.z,
					GetNodeData(Position + VS_EAST_OFFSET),
					GetNodeData(Position + VS_WEST_OFFSET),
					GetNodeData(Position + VS_TOP_OFFSET),
					GetNodeData(Position + VS_BOTTOM_OFFSET),
					GetNodeData(Position + VS_NORTH_OFFSET),
					GetNodeData(Position + VS_SOUTH_OFFSET),
					true
					);

				IsRenderStateDirty = true;
			}

			if (Node->Location != 1)
			{
				OctreeNode* ParentNode = Nodes[(Node->Location >> 3)];

				// Set the children of the parent to this octant
				uint32_t Octant = Node->Location & 7;
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
			Voxel* OldData = Node->NodeData;
			Node->NodeData = NULL;

			glm::uvec3 OldPosition = glm::uvec3(Node->ChunkPosition);
			uint32_t OldNodeOctant = Node->GetOctantForPosition(OldPosition);
			uint32_t NewNodeOctant = Node->GetOctantForPosition(Position);

			if (OldNodeOctant == NewNodeOctant)
			{
				// They need to go in the same octant so add one of them
				// and let the recursion do the rest
				OctreeNode* OldNode = new OctreeNode();
				OldNode->Location = (Node->Location << 3) | OldNodeOctant;
				OldNode->Size = HalfSize;
				OldNode->Center = Node->Center;
				OldNode->Center.x += ((int32_t) QuarterSize) * ((OldNodeOctant & 4) ? 1 : -1);
				OldNode->Center.y += ((int32_t) QuarterSize) * ((OldNodeOctant & 2) ? 1 : -1);
				OldNode->Center.z += ((int32_t) QuarterSize) * ((OldNodeOctant & 1) ? 1 : -1);

				Nodes.insert({ OldNode->Location, OldNode });

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
				OldNode->Center.x += ((int32_t) QuarterSize) * ((OldNodeOctant & 4) ? 1 : -1);
				OldNode->Center.y += ((int32_t) QuarterSize) * ((OldNodeOctant & 2) ? 1 : -1);
				OldNode->Center.z += ((int32_t) QuarterSize) * ((OldNodeOctant & 1) ? 1 : -1);
				Nodes.insert({ OldNode->Location, OldNode });

				OctreeNode* NewNode = new OctreeNode();
				NewNode->Location = (Node->Location << 3) | NewNodeOctant;
				NewNode->Size = HalfSize;
				NewNode->Center = Node->Center;
				NewNode->Center.x += ((int32_t) QuarterSize) * ((NewNodeOctant & 4) ? 1 : -1);
				NewNode->Center.y += ((int32_t) QuarterSize) * ((NewNodeOctant & 2) ? 1 : -1);
				NewNode->Center.z += ((int32_t) QuarterSize) * ((NewNodeOctant & 1) ? 1 : -1);
				Nodes.insert({ NewNode->Location, NewNode });

				Node->Children |= (1 << OldNodeOctant);
				Node->Children |= (1 << NewNodeOctant);

				InsertNode(OldPosition, OldData, OldNode, false);
				InsertNode(Position, NewVoxel, NewNode, true);
			}
		}
	}
	else
	{
		uint32_t NodeOctant = Node->GetOctantForPosition(Position);

		if ((Node->Children >> NodeOctant) & 1) // It has a child in that octant
		{
			InsertNode(Position, NewVoxel, Nodes[(Node->Location << 3) | NodeOctant]);
		}
		else // It doesn't have a node in that octant, create it
		{
			OctreeNode* NewNode = new OctreeNode();
			NewNode->Location = (Node->Location << 3) | NodeOctant;
			NewNode->Size = HalfSize;
			NewNode->Center = Node->Center;
			NewNode->Center.x += ((int32_t) QuarterSize) * ((NodeOctant & 4) ? 1 : -1);
			NewNode->Center.y += ((int32_t) QuarterSize) * ((NodeOctant & 2) ? 1 : -1);
			NewNode->Center.z += ((int32_t) QuarterSize) * ((NodeOctant & 1) ? 1 : -1);
			Node->Children |= (1 << NodeOctant);
			Nodes.insert({ NewNode->Location, NewNode });

			InsertNode(Position, NewVoxel, NewNode, true);
		}
	}
}

OctreeNode* Chunk::GetNode(uint32_t Position)
{
	const auto iter = Nodes.find(Position);
	if (iter == Nodes.end())
		return nullptr;
	else
		return (*iter).second;
}

__forceinline OctreeNode* Chunk::GetNode(const glm::uvec3& Position, OctreeNode* Node)
{
	if (!IsWithinBounds(Position, Extent))
	{
		return NULL;
	}
	else
	{
		if (Node->Children != 0 && Node->Size != 1)
		{
			uint32_t Octant = Node->GetOctantForPosition(Position);

			if ((Node->Children >> Octant) & 1)
			{
				return GetNode(Position, Nodes.at((Node->Location << 3) | Octant));
			}
		}
		else
		{
			if (Node->ChunkPosition == Position)
			{
				return Node;
			}
		}
	}
	return NULL;
}

void Chunk::RemoveNode(const glm::uvec3& Position, OctreeNode* Node, bool IsUpwardsRecursive)
{
	OctreeNode* NodeToBeDeleted = GetNode(Position, Node);
	if (NodeToBeDeleted)
	{
		if (NodeToBeDeleted->Children != 0 && !IsUpwardsRecursive)
		{
			// We are not a leaf node and we must therefore continue the search 
			// in this nodes children
			uint32_t Octant = NodeToBeDeleted->GetOctantForPosition(Position);

			// If it has a node in the octant we are looking for try removing
			// that one instead. Otherwise, the n
			if ((Node->Children >> Octant) & 1)
			{
				RemoveNode(Position, Nodes.at((Node->Location << 3) | Octant));
			}
		}
		else if (NodeToBeDeleted->Location != 1) // We are not the RootNode
		{
			uint32_t NodeOctant = NodeToBeDeleted->Location & 7;
			OctreeNode* ParentNode = Nodes[(NodeToBeDeleted->Location >> 3)];
			ParentNode->Children ^= (1 << NodeOctant); // Remove from parents children
			NodeToBeDeleted->NodeData->OnNodeUpdatedAdjacent(
				(uint8_t) Position.x,
				(uint8_t) Position.y,
				(uint8_t) Position.z,
				GetNodeData(Position + VS_EAST_OFFSET),
				GetNodeData(Position + VS_WEST_OFFSET),
				GetNodeData(Position + VS_TOP_OFFSET),
				GetNodeData(Position + VS_BOTTOM_OFFSET),
				GetNodeData(Position + VS_NORTH_OFFSET),
				GetNodeData(Position + VS_SOUTH_OFFSET),
				false
				);
			//RemoveNode(AddData, RemoveData, ParentNode->ChunkPosition, ParentNode, true);

			Nodes.erase(NodeToBeDeleted->Location);
			//TODO: Check if all the sister nodes are empty - if so, remove the parent node aswell
			delete NodeToBeDeleted;
		}
	}
}

void Chunk::Update()
{
	if (ContainsElementsToRemove)
	{
		while (ElementsToRemove.GetNum() > 0)
		{
			RemoveNode(ElementsToRemove.Pop(), RootNode);
		}
		ContainsElementsToRemove = false;
	}
	if (ContainsElementsToAdd)
	{
		while (ElementsToAdd.GetNum() > 0)
		{
			VoxelAddData Data = ElementsToAdd.Pop();
			InsertNode(Data.Position, Data.Value, RootNode);
		}
		ContainsElementsToAdd = false;
	}
	if (IsRenderStateDirty)
	{
		if (RenderData)
			delete RenderData;
		
		RenderData = ChunkRenderer::CreateRenderData(glm::vec3(ChunkX * Chunk::SIZE, ChunkY * Chunk::SIZE, ChunkZ * Chunk::SIZE), this);
		
		IsRenderStateDirty = false;
	}
}

Chunk::Chunk()
{
	// Insert the root node
	RootNode = new OctreeNode();
	RootNode->Location = 1;
	RootNode->Size = 1 << Chunk::DEPTH;

	uint32_t HalfSize = RootNode->Size >> 1;
	RootNode->Center = glm::uvec3(HalfSize, HalfSize, HalfSize);

	Extent = glm::uvec3(RootNode->Size, RootNode->Size, RootNode->Size);

	Nodes.insert({ 0b1, RootNode });

	RenderData = NULL;
}

Chunk::~Chunk()
{
	Nodes.clear();
	RootNode = NULL;

	//TODO: Tell the chunk manager to remove this chunk
}
