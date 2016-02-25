#include "Octree.h"

#include <intrin.h>

#include "../Rendering/ChunkRenderComponent.h"

glm::uvec3 Chunk::VS_EAST_OFFSET = glm::uvec3(1, 0, 0);
glm::uvec3 Chunk::VS_WEST_OFFSET = glm::uvec3(-1, 0, 0);
glm::uvec3 Chunk::VS_TOP_OFFSET	= glm::uvec3(0, 1, 0);
glm::uvec3 Chunk::VS_BOTTOM_OFFSET = glm::uvec3(0, -1, 0);
glm::uvec3 Chunk::VS_NORTH_OFFSET = glm::uvec3(0, 0, 1);
glm::uvec3 Chunk::VS_SOUTH_OFFSET = glm::uvec3(0, 0, -1);

uint32_t Chunk::DEPTH = 7; // Chunk size is 2^7=128
float Voxel::CUBE_SIZE = 1.0f;

void Voxel::OnNodePlacedAdjacent(Voxel* NodeEast, Voxel* NodeWest, Voxel* NodeTop, Voxel* NodeBottom, Voxel* NodeNorth, Voxel* NodeSouth)
{
	uint8_t Result = 63;
	if (NodeEast)
	{
		Result ^= VoxelSide::SIDE_EAST;
		NodeEast->OnNodePlacedOnSide(VoxelSide::SIDE_WEST);
	}
	if (NodeWest)
	{
		Result ^= VoxelSide::SIDE_WEST;
		NodeWest->OnNodePlacedOnSide(VoxelSide::SIDE_EAST);
	}
	if (NodeTop)
	{
		Result ^= VoxelSide::SIDE_TOP;
		NodeTop->OnNodePlacedOnSide(VoxelSide::SIDE_BOTTOM);
	}
	if (NodeBottom)
	{
		Result ^= VoxelSide::SIDE_BOTTOM;
		NodeBottom->OnNodePlacedOnSide(VoxelSide::SIDE_TOP);
	}
	if (NodeNorth)
	{
		Result ^= VoxelSide::SIDE_NORTH;
		NodeNorth->OnNodePlacedOnSide(VoxelSide::SIDE_SOUTH);
	}
	if (NodeSouth)
	{
		Result ^= VoxelSide::SIDE_SOUTH;
		NodeSouth->OnNodePlacedOnSide(VoxelSide::SIDE_NORTH);
	}
	SidesToRender = Result;
}

__forceinline void Voxel::OnNodePlacedOnSide(const VoxelSide& Side)
{
	if ((SidesToRender & Side) == Side)
	{
		SidesToRender ^= Side;
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

				if (Node->Location != 1)
				{
					Node->NodeData->OnNodePlacedAdjacent(
						GetNodeData(Position + VS_EAST_OFFSET),
						GetNodeData(Position + VS_WEST_OFFSET), 
						GetNodeData(Position + VS_TOP_OFFSET),
						GetNodeData(Position + VS_BOTTOM_OFFSET),
						GetNodeData(Position + VS_NORTH_OFFSET),
						GetNodeData(Position + VS_SOUTH_OFFSET)
						);
				}

				UpdateRenderInformation();
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
				return GetNode(Position, Nodes[(Node->Location << 3) | Octant]);
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
			//TODO: Here we need to visit every child and add them to a list to then re-add them
			// to the tree once this node is gone
		}
		else if (Node->Location != 1) // We are not the RootNode
		{
			uint32_t NodeOctant = Node->Location & 7;
			OctreeNode* ParentNode = Nodes[(Node->Location >> 3)];
			ParentNode->Children ^= (1 << NodeOctant); // Remove from parents children
			RemoveNode(ParentNode->ChunkPosition, ParentNode, true);

			Nodes.erase(NodeToBeDeleted->Location);
			// Check if all the sister nodes are empty - if so, remove the parent node aswell
			delete NodeToBeDeleted;
		}
	}
}

inline void Chunk::UpdateRenderInformation()
{
	std::vector<VoxelVertex> Vertices;
	Vertices.reserve(Nodes.size() * 6 * 6);
	for (auto It = Nodes.begin(); It != Nodes.end(); ++It)
	{
		Voxel* VoxelToRender = (*It).second->NodeData;

		if (VoxelToRender && VoxelToRender->SidesToRender)
		{
			glm::vec3 VoxelPosition = (*It).second->ChunkPosition;
			glm::vec3 Vertex1 = VoxelPosition;
			glm::vec3 Vertex2 = VoxelPosition + glm::vec3(0.0f, Voxel::CUBE_SIZE, 0.0f);
			glm::vec3 Vertex3 = VoxelPosition + glm::vec3(Voxel::CUBE_SIZE, Voxel::CUBE_SIZE, 0.0f);
			glm::vec3 Vertex4 = VoxelPosition + glm::vec3(Voxel::CUBE_SIZE, 0, 0.0f);
			glm::vec3 Vertex5 = VoxelPosition + glm::vec3(0.0f, 0.0f, -Voxel::CUBE_SIZE);
			glm::vec3 Vertex6 = VoxelPosition + glm::vec3(0.0f, Voxel::CUBE_SIZE, -Voxel::CUBE_SIZE);
			glm::vec3 Vertex7 = VoxelPosition + glm::vec3(Voxel::CUBE_SIZE, Voxel::CUBE_SIZE, -Voxel::CUBE_SIZE);
			glm::vec3 Vertex8 = VoxelPosition + glm::vec3(Voxel::CUBE_SIZE, 0.0f, -Voxel::CUBE_SIZE);

			if ((VoxelToRender->SidesToRender & VoxelSide::SIDE_EAST) == VoxelSide::SIDE_EAST)
			{
				Vertices.push_back({ Vertex4, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
				Vertices.push_back({ Vertex3, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
				Vertices.push_back({ Vertex7, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
				Vertices.push_back({ Vertex4, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
				Vertices.push_back({ Vertex7, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
				Vertices.push_back({ Vertex8, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
			}
			if ((VoxelToRender->SidesToRender & VoxelSide::SIDE_WEST) == VoxelSide::SIDE_WEST)
			{
				Vertices.push_back({ Vertex5, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
				Vertices.push_back({ Vertex6, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
				Vertices.push_back({ Vertex2, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
				Vertices.push_back({ Vertex5, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
				Vertices.push_back({ Vertex2, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
				Vertices.push_back({ Vertex1, VoxelToRender->ColorRed, VoxelToRender->ColorGreen, VoxelToRender->ColorBlue });
			}
			if ((VoxelToRender->SidesToRender & VoxelSide::SIDE_TOP) == VoxelSide::SIDE_TOP)
			{
				//glVertex3fv(&Vertex2.x);
				//glVertex3fv(&Vertex6.x);
				//glVertex3fv(&Vertex7.x);
				//glVertex3fv(&Vertex3.x);
			}
			if ((VoxelToRender->SidesToRender & VoxelSide::SIDE_BOTTOM) == VoxelSide::SIDE_BOTTOM)
			{
				//glVertex3fv(&Vertex1.x);
				//glVertex3fv(&Vertex5.x);
				//glVertex3fv(&Vertex8.x);
				//glVertex3fv(&Vertex4.x);
			}
			if ((VoxelToRender->SidesToRender & VoxelSide::SIDE_NORTH) == VoxelSide::SIDE_NORTH)
			{
				//glVertex3fv(&Vertex8.x);
				//glVertex3fv(&Vertex7.x);
				//glVertex3fv(&Vertex6.x);
				//glVertex3fv(&Vertex5.x);
			}
			if ((VoxelToRender->SidesToRender & VoxelSide::SIDE_SOUTH) == VoxelSide::SIDE_SOUTH)
			{
				//glVertex3fv(&Vertex1.x);
				//glVertex3fv(&Vertex2.x);
				//glVertex3fv(&Vertex3.x);
				//glVertex3fv(&Vertex4.x);
			}
		}
	}
	m_RenderComponent->SetData(Vertices);
}

void Chunk::Render()
{
	m_RenderComponent->Render(0.0f);
}

Chunk::Chunk()
{
	// Insert the root node
	RootNode = new OctreeNode();
	RootNode->Location = 1;
	RootNode->Size = Chunk::DEPTH;

	uint32_t HalfSize = Chunk::DEPTH >> 1;
	RootNode->Center = glm::uvec3(HalfSize, HalfSize, HalfSize);

	Extent = glm::uvec3(Chunk::DEPTH, Chunk::DEPTH, Chunk::DEPTH);

	Nodes.insert({ 0b1, RootNode });

	m_RenderComponent = new ChunkRenderComponent();
}

Chunk::~Chunk()
{
	Nodes.clear();
	RootNode = NULL;

	delete m_RenderComponent;
	m_RenderComponent = NULL;

	//TODO: Tell the chunk manager to remove this chunk
}
