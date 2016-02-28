#include "Octree.h"

#include <intrin.h>

glm::uvec3 Chunk::VS_EAST_OFFSET = glm::uvec3(1, 0, 0);
glm::uvec3 Chunk::VS_WEST_OFFSET = glm::uvec3(-1, 0, 0);
glm::uvec3 Chunk::VS_TOP_OFFSET = glm::uvec3(0, 1, 0);
glm::uvec3 Chunk::VS_BOTTOM_OFFSET = glm::uvec3(0, -1, 0);
glm::uvec3 Chunk::VS_NORTH_OFFSET = glm::uvec3(0, 0, 1);
glm::uvec3 Chunk::VS_SOUTH_OFFSET = glm::uvec3(0, 0, -1);

uint32_t Chunk::DEPTH = 7; // Chunk size is 2^7=128
float Voxel::CUBE_SIZE = 1.0f;

void Voxel::OnNodeUpdatedAdjacent(VoxelBufferData* AddData, VoxelBufferData* RemoveData, const uint8_t& X, const uint8_t& Y, const uint8_t& Z, Voxel* NodeEast, Voxel* NodeWest, Voxel* NodeTop, Voxel* NodeBottom, Voxel* NodeNorth, Voxel* NodeSouth, const bool& Placed)
{
	uint8_t Result = 63;
	ChunkRenderCoordinate Coord = ChunkRenderCoordinate(X, Y, Z);
	if (NodeEast)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_EAST;
		NodeEast->OnNodeUpdatedOnSide(AddData, RemoveData, X + 1, Y, Z, VoxelSide::SIDE_WEST, Placed);
	}
	else
	{
		auto It = std::find(RemoveData->EastFaces.begin(), RemoveData->EastFaces.end(), Coord);
		if (It != RemoveData->EastFaces.end())
			RemoveData->EastFaces.erase(It);
		AddData->EastFaces.push_back(Coord);
	}
	if (NodeWest)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_WEST;
		NodeWest->OnNodeUpdatedOnSide(AddData, RemoveData, X - 1, Y, Z, VoxelSide::SIDE_EAST, Placed);
	}
	else
	{
		auto It = std::find(RemoveData->WestFaces.begin(), RemoveData->WestFaces.end(), Coord);
		if (It != RemoveData->WestFaces.end())
			RemoveData->WestFaces.erase(It);
		AddData->WestFaces.push_back(Coord);
	}
	if (NodeTop)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_TOP;
		NodeTop->OnNodeUpdatedOnSide(AddData, RemoveData, X, Y + 1, Z, VoxelSide::SIDE_BOTTOM, Placed);
	}
	else
	{
		auto It = std::find(RemoveData->TopFaces.begin(), RemoveData->TopFaces.end(), Coord);
		if (It != RemoveData->TopFaces.end())
			RemoveData->TopFaces.erase(It);
		AddData->TopFaces.push_back(Coord);
	}
	if (NodeBottom)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_BOTTOM;
		NodeBottom->OnNodeUpdatedOnSide(AddData, RemoveData, X, Y - 1, Z, VoxelSide::SIDE_TOP, Placed);
	}
	else
	{
		auto It = std::find(RemoveData->BottomFaces.begin(), RemoveData->BottomFaces.end(), Coord);
		if (It != RemoveData->BottomFaces.end())
			RemoveData->BottomFaces.erase(It);
		AddData->BottomFaces.push_back(Coord);
	}
	if (NodeNorth)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_NORTH;
		NodeNorth->OnNodeUpdatedOnSide(AddData, RemoveData, X, Y, Z + 1, VoxelSide::SIDE_SOUTH, Placed);
	}
	else
	{
		auto It = std::find(RemoveData->NorthFaces.begin(), RemoveData->NorthFaces.end(), Coord);
		if (It != RemoveData->NorthFaces.end())
			RemoveData->NorthFaces.erase(It);
		AddData->NorthFaces.push_back(Coord);
	}
	if (NodeSouth)
	{
		if (Placed)
			Result ^= VoxelSide::SIDE_SOUTH;
		NodeSouth->OnNodeUpdatedOnSide(AddData, RemoveData, X, Y, Z - 1, VoxelSide::SIDE_NORTH, Placed);
	}
	else
	{
		auto It = std::find(RemoveData->SouthFaces.begin(), RemoveData->SouthFaces.end(), Coord);
		if (It != RemoveData->SouthFaces.end())
			RemoveData->SouthFaces.erase(It);
		AddData->SouthFaces.push_back(Coord);
	}
	if (!Placed)
	{
		SidesToRender = 0;
		RemoveData->EastFaces.push_back(Coord);
		RemoveData->WestFaces.push_back(Coord);
		RemoveData->TopFaces.push_back(Coord);
		RemoveData->BottomFaces.push_back(Coord);
		RemoveData->NorthFaces.push_back(Coord);
		RemoveData->SouthFaces.push_back(Coord);
	}
	else
	{
		SidesToRender = Result;
	}
}

// Placed means that the original block which called this function was placed not removed
__forceinline void Voxel::OnNodeUpdatedOnSide(VoxelBufferData* AddData, VoxelBufferData* RemoveData, const uint8_t& X, const uint8_t& Y, const uint8_t& Z, const VoxelSide& Side, const bool& Placed)
{
	// Don't render on this side anymore
	if ((SidesToRender & Side) == Side)
	{
		SidesToRender ^= Side;
		switch (Side)
		{
			case SIDE_EAST:
			{
				ChunkRenderCoordinate Coord = ChunkRenderCoordinate(X, Y, Z);
				auto It = std::find(AddData->EastFaces.begin(), AddData->EastFaces.end(), Coord);
				if (It != AddData->EastFaces.end())
					if (Placed)
						AddData->EastFaces.erase(It);
					else
						AddData->EastFaces.push_back(Coord);
				else
					RemoveData->EastFaces.push_back(Coord);
				break;
			}
			case SIDE_WEST:
			{
				ChunkRenderCoordinate Coord = ChunkRenderCoordinate(X, Y, Z);
				auto It = std::find(AddData->WestFaces.begin(), AddData->WestFaces.end(), Coord);
				if (It != AddData->WestFaces.end())
					if (Placed)
						AddData->WestFaces.erase(It);
					else
						AddData->WestFaces.push_back(Coord);
				else
					RemoveData->WestFaces.push_back(Coord);
				break;
			}
			case SIDE_TOP:
			{
				ChunkRenderCoordinate Coord = ChunkRenderCoordinate(X, Y, Z);
				auto It = std::find(AddData->TopFaces.begin(), AddData->TopFaces.end(), Coord);
				if (It != AddData->TopFaces.end())
					if (Placed)
						AddData->TopFaces.erase(It);
					else
						AddData->TopFaces.push_back(Coord);
				else
					RemoveData->TopFaces.push_back(Coord);
				break;
			}
			case SIDE_BOTTOM:
			{
				ChunkRenderCoordinate Coord = ChunkRenderCoordinate(X, Y, Z);
				auto It = std::find(AddData->BottomFaces.begin(), AddData->BottomFaces.end(), Coord);
				if (It != AddData->BottomFaces.end())
					if (Placed)
						AddData->BottomFaces.erase(It);
					else
						AddData->BottomFaces.push_back(Coord);
				else
					RemoveData->BottomFaces.push_back(Coord);
				break;
			}
			case SIDE_NORTH:
			{
				ChunkRenderCoordinate Coord = ChunkRenderCoordinate(X, Y, Z);
				auto It = std::find(AddData->NorthFaces.begin(), AddData->NorthFaces.end(), Coord);
				if (It != AddData->NorthFaces.end())
					if (Placed)
						AddData->NorthFaces.erase(It);
					else
						AddData->NorthFaces.push_back(Coord);
				else
					RemoveData->NorthFaces.push_back(Coord);
				break;
			}
			case SIDE_SOUTH:
			{
				ChunkRenderCoordinate Coord = ChunkRenderCoordinate(X, Y, Z);
				auto It = std::find(AddData->SouthFaces.begin(), AddData->SouthFaces.end(), Coord);
				if (It != AddData->SouthFaces.end())
					if (Placed)
						AddData->SouthFaces.erase(It);
					else
						AddData->SouthFaces.push_back(Coord);
				else
					RemoveData->SouthFaces.push_back(Coord);
				break;
			}
		}
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

void Chunk::InsertNode(VoxelBufferData* AddData, VoxelBufferData* RemoveData, const glm::uvec3& Position, Voxel* NewVoxel, OctreeNode* Node, bool IsNew)
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
					AddData,
					RemoveData,
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
				InsertNode(AddData, RemoveData, OldPosition, OldData, OldNode, false);
				InsertNode(AddData, RemoveData, Position, NewVoxel, OldNode, true);
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

				InsertNode(AddData, RemoveData, OldPosition, OldData, OldNode, false);
				InsertNode(AddData, RemoveData, Position, NewVoxel, NewNode, true);
			}
		}
	}
	else
	{
		uint32_t NodeOctant = Node->GetOctantForPosition(Position);

		if ((Node->Children >> NodeOctant) & 1) // It has a child in that octant
		{
			InsertNode(AddData, RemoveData, Position, NewVoxel, Nodes[(Node->Location << 3) | NodeOctant]);
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

			InsertNode(AddData, RemoveData, Position, NewVoxel, NewNode, true);
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

void Chunk::RemoveNode(VoxelBufferData* AddData, VoxelBufferData* RemoveData, const glm::uvec3& Position, OctreeNode* Node, bool IsUpwardsRecursive)
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
				RemoveNode(AddData, RemoveData, Position, Nodes.at((Node->Location << 3) | Octant));
			}
		}
		else if (NodeToBeDeleted->Location != 1) // We are not the RootNode
		{
			uint32_t NodeOctant = NodeToBeDeleted->Location & 7;
			OctreeNode* ParentNode = Nodes[(NodeToBeDeleted->Location >> 3)];
			ParentNode->Children ^= (1 << NodeOctant); // Remove from parents children
			NodeToBeDeleted->NodeData->OnNodeUpdatedAdjacent(
				AddData, 
				RemoveData, 
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
	VoxelBufferData* AddData = new VoxelBufferData();
	VoxelBufferData* RemoveData = new VoxelBufferData();

	if (ContainsElementsToRemove)
	{
		while (ElementsToRemove.GetNum() > 0)
		{
			RemoveNode(AddData, RemoveData, ElementsToRemove.Pop(), RootNode);
		}
		ContainsElementsToRemove = false;
	}
	if (ContainsElementsToAdd)
	{
		while (ElementsToAdd.GetNum() > 0)
		{
			VoxelAddData Data = ElementsToAdd.Pop();
			InsertNode(AddData, RemoveData, Data.Position, Data.Value, RootNode);
		}
		ContainsElementsToAdd = false;
	}
	if (IsRenderStateDirty)
	{
		if (RenderData == NULL)
		{
			RenderData = ChunkRenderer::CreateRenderData(glm::vec3(0.0f, 0.0f, 0.0f));
		}

		for (auto& It : RemoveData->EastFaces)
		{
			ChunkRenderer::SpliceFromBufferSide(RenderData, SIDE_EAST, It);
		}
		for (auto& It : RemoveData->WestFaces)
		{
			ChunkRenderer::SpliceFromBufferSide(RenderData, SIDE_WEST, It);
		}
		for (auto& It : RemoveData->TopFaces)
		{
			ChunkRenderer::SpliceFromBufferSide(RenderData, SIDE_TOP, It);
		}
		for (auto& It : RemoveData->BottomFaces)
		{
			ChunkRenderer::SpliceFromBufferSide(RenderData, SIDE_BOTTOM, It);
		}
		for (auto& It : RemoveData->NorthFaces)
		{
			ChunkRenderer::SpliceFromBufferSide(RenderData, SIDE_NORTH, It);
		}
		for (auto& It : RemoveData->SouthFaces)
		{
			ChunkRenderer::SpliceFromBufferSide(RenderData, SIDE_SOUTH, It);
		}

		ChunkRenderer::InsertBatchIntoBufferSide(RenderData, SIDE_EAST, &AddData->EastFaces[0], (uint32_t) AddData->EastFaces.size());
		ChunkRenderer::InsertBatchIntoBufferSide(RenderData, SIDE_WEST, &AddData->WestFaces[0], (uint32_t) AddData->WestFaces.size());
		ChunkRenderer::InsertBatchIntoBufferSide(RenderData, SIDE_TOP, &AddData->TopFaces[0], (uint32_t) AddData->TopFaces.size());
		ChunkRenderer::InsertBatchIntoBufferSide(RenderData, SIDE_BOTTOM, &AddData->BottomFaces[0], (uint32_t) AddData->BottomFaces.size());
		ChunkRenderer::InsertBatchIntoBufferSide(RenderData, SIDE_NORTH, &AddData->NorthFaces[0], (uint32_t) AddData->NorthFaces.size());
		ChunkRenderer::InsertBatchIntoBufferSide(RenderData, SIDE_SOUTH, &AddData->SouthFaces[0], (uint32_t) AddData->SouthFaces.size());

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
