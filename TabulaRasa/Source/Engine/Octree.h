#pragma once

#include <unordered_map>
#include <stdint.h>

#include "glm/common.hpp"

#include "../Engine/Engine.h"

FORCEINLINE bool IsWithinBounds(const glm::uvec3& Position, const glm::uvec3& Bounds) //TODO: SIMD optimize from custom vector class
{
	if ((Position.x < Bounds.x) &&
		(Position.y < Bounds.y) &&
		(Position.z < Bounds.z))
		return true;
	else
		return false;
}

FORCEINLINE bool IsContainedWithin(const glm::uvec3& Position, const glm::uvec3& Min, const glm::uvec3& Max) //TODO: SIMD optimize from custom vector class
{
	if ((Position.x < Max.x) && (Position.x >= Min.x) &&
		(Position.y < Max.y) && (Position.y >= Min.y) &&
		(Position.z < Max.z) && (Position.z >= Min.z))
		return true;
	else
		return false;
}

template<class T>
struct OctreeNode
{
	FORCEINLINE OctreeNode() : m_pNodeData(NULL), m_Children(0), m_Location(0), m_Size(0) {}

	FORCEINLINE ~OctreeNode() { if (m_pNodeData) { delete m_pNodeData; m_pNodeData = NULL; } }

	size_t GetOctantForPosition(const glm::uvec3& Position);

	T*			m_pNodeData;
	uint8_t		m_Children;

	uint32_t	m_Location;
	uint32_t	m_Size;

	glm::uvec3	m_LocalPosition;
	glm::uvec3	m_Center;
};

template<typename T>
FORCEINLINE size_t OctreeNode<T>::GetOctantForPosition(const glm::uvec3& Position)
{
	size_t Result = 0;
	//uint32_t HalfSize = m_Size >> 1;
	if (Position.x >= m_Center.x) Result |= 4;
	if (Position.y >= m_Center.y) Result |= 2;
	if (Position.z >= m_Center.z) Result |= 1;
	return Result;
}

template<typename T>
class Octree
{
public:

	friend class World;

	Octree()
	{
		// Insert the root node
		m_pRootNode = new OctreeNode<T>();
		m_pRootNode->m_Location = 1;
		m_pRootNode->m_Size = 1 << Octree::DEPTH;

		uint32_t HalfSize = m_pRootNode->m_Size >> 1;
		m_pRootNode->m_Center = glm::uvec3(HalfSize, HalfSize, HalfSize);

		m_Extent = glm::uvec3(m_pRootNode->m_Size, m_pRootNode->m_Size, m_pRootNode->m_Size);

		Nodes.insert({ 0b1, m_pRootNode });
	}

	~Octree() {	{ Nodes.clear(); m_pRootNode = NULL; } }

	FORCEINLINE void InsertNode(const glm::uvec3& Position, T* NewData)
	{
		InsertNode(Position, NewData, m_pRootNode);
	}

	FORCEINLINE void RemoveNode(const glm::uvec3& Position)
	{
		RemoveNode(Position, m_pRootNode);
	}

	OctreeNode<T>* Get(const glm::uvec3& Position, OctreeNode<T>* Node);

	OctreeNode<T>* GetNode(const glm::uvec3& Position)
	{
		return Get(Position, m_pRootNode);
	}

	void RemoveNode(const glm::uvec3& Position, OctreeNode<T>* Node, bool IsUpwardsRecursive = false);

	void InsertNode(const glm::uvec3& Position, T* NewData, OctreeNode<T>* Node, bool IsNew = true);

	FORCEINLINE T* GetNodeData(const glm::uvec3& Position)
	{
		OctreeNode<T>* Node = Get(Position, m_pRootNode);
		return Node ? Node->m_pNodeData : NULL;
	}

	FORCEINLINE T* GetNodeData(const glm::ivec3& Position)
	{
		OctreeNode<T>* Node = Get(glm::uvec3(Position.x, Position.y, Position.z), m_pRootNode);
		return Node ? Node->m_pNodeData : NULL;
	}

public:

	static constexpr uint32_t DEPTH = 5; // Chunk size is 2^5=32

	OctreeNode<T>* m_pRootNode;

	glm::uvec3 m_Extent;

	std::unordered_map<uint32_t, OctreeNode<T>*> Nodes;
};

template<typename T>
void Octree<T>::InsertNode(const glm::uvec3& Position, T* NewData, OctreeNode<T>* Node, bool IsNew)
{
	uint32_t HalfSize = Node->m_Size >> 1;
	uint32_t QuarterSize = HalfSize >> 1;
	if (Node->m_Children == 0)
	{
		if (Node->m_pNodeData == NULL || Node->m_Size == 1)
		{
			Node->m_pNodeData = NewData;
			Node->m_LocalPosition = Position;

			if (Node->m_Location != 1)
			{
				OctreeNode<T>* ParentNode = Nodes[(Node->m_Location >> 3)];

				// Set the m_Children of the parent to this octant
				uint32_t Octant = Node->m_Location & 7;
				ParentNode->m_Children |= (1 << Octant);
			}
		}
		else
		{
			// There already exists something at this node
			// and it needs to get split
			T* OldData = Node->m_pNodeData;
			Node->m_pNodeData = NULL;

			glm::uvec3 OldPosition = glm::uvec3(Node->m_LocalPosition);
			uint32_t OldNodeOctant = Node->GetOctantForPosition(OldPosition);
			uint32_t NewNodeOctant = Node->GetOctantForPosition(Position);

			if (OldNodeOctant == NewNodeOctant)
			{
				// They need to go in the same octant so add one of them
				// and let the recursion do the rest
				OctreeNode<T>* OldNode = new OctreeNode<T>();
				OldNode->m_Location = (Node->m_Location << 3) | OldNodeOctant;
				OldNode->m_Size = HalfSize;
				OldNode->m_Center = Node->m_Center;
				OldNode->m_Center.x += ((int32_t)QuarterSize) * ((OldNodeOctant & 4) ? 1 : -1);
				OldNode->m_Center.y += ((int32_t)QuarterSize) * ((OldNodeOctant & 2) ? 1 : -1);
				OldNode->m_Center.z += ((int32_t)QuarterSize) * ((OldNodeOctant & 1) ? 1 : -1);

				Nodes.insert({ OldNode->m_Location, OldNode });

				Node->m_Children |= (1 << OldNodeOctant);
				InsertNode(OldPosition, OldData, OldNode, false);
				InsertNode(Position, NewData, OldNode, true);
			}
			else
			{
				OctreeNode<T>* OldNode = new OctreeNode<T>();
				OldNode->m_Location = (Node->m_Location << 3) | OldNodeOctant;
				OldNode->m_Size = HalfSize;
				OldNode->m_Center = Node->m_Center;
				OldNode->m_Center.x += ((int32_t)QuarterSize) * ((OldNodeOctant & 4) ? 1 : -1);
				OldNode->m_Center.y += ((int32_t)QuarterSize) * ((OldNodeOctant & 2) ? 1 : -1);
				OldNode->m_Center.z += ((int32_t)QuarterSize) * ((OldNodeOctant & 1) ? 1 : -1);
				Nodes.insert({ OldNode->m_Location, OldNode });

				OctreeNode<T>* NewNode = new OctreeNode<T>();
				NewNode->m_Location = (Node->m_Location << 3) | NewNodeOctant;
				NewNode->m_Size = HalfSize;
				NewNode->m_Center = Node->m_Center;
				NewNode->m_Center.x += ((int32_t)QuarterSize) * ((NewNodeOctant & 4) ? 1 : -1);
				NewNode->m_Center.y += ((int32_t)QuarterSize) * ((NewNodeOctant & 2) ? 1 : -1);
				NewNode->m_Center.z += ((int32_t)QuarterSize) * ((NewNodeOctant & 1) ? 1 : -1);
				Nodes.insert({ NewNode->m_Location, NewNode });

				Node->m_Children |= (1 << OldNodeOctant);
				Node->m_Children |= (1 << NewNodeOctant);

				InsertNode(OldPosition, OldData, OldNode, false);
				InsertNode(Position, NewData, NewNode, true);
			}
		}
	}
	else
	{
		uint32_t NodeOctant = Node->GetOctantForPosition(Position);

		if ((Node->m_Children >> NodeOctant) & 1) // It has a child in that octant
		{
			InsertNode(Position, NewData, Nodes[(Node->m_Location << 3) | NodeOctant]);
		}
		else // It doesn't have a node in that octant, create it
		{
			OctreeNode<T>* NewNode = new OctreeNode<T>();
			NewNode->m_Location = (Node->m_Location << 3) | NodeOctant;
			NewNode->m_Size = HalfSize;
			NewNode->m_Center = Node->m_Center;
			NewNode->m_Center.x += ((int32_t)QuarterSize) * ((NodeOctant & 4) ? 1 : -1);
			NewNode->m_Center.y += ((int32_t)QuarterSize) * ((NodeOctant & 2) ? 1 : -1);
			NewNode->m_Center.z += ((int32_t)QuarterSize) * ((NodeOctant & 1) ? 1 : -1);
			Node->m_Children |= (1 << NodeOctant);
			Nodes.insert({ NewNode->m_Location, NewNode });

			InsertNode(Position, NewData, NewNode, true);
		}
	}
}

template<typename T>
OctreeNode<T>* Octree<T>::Get(const glm::uvec3& Position, OctreeNode<T>* Node)
{
	if (!IsWithinBounds(Position, m_Extent))
	{
		return NULL;
	}
	else
	{
		if (Node->m_Children != 0 && Node->m_Size != 1)
		{
			uint32_t Octant = Node->GetOctantForPosition(Position);

			if ((Node->m_Children >> Octant) & 1)
			{
				return Get(Position, (OctreeNode<T>*) Nodes.at((Node->m_Location << 3) | Octant));
			}
		}
		else
		{
			if (Node->m_LocalPosition == Position)
			{
				return Node;
			}
		}
	}
	return NULL;
}

template<typename T>
void Octree<T>::RemoveNode(const glm::uvec3& Position, OctreeNode<T>* Node, bool IsUpwardsRecursive)
{
	OctreeNode<T>* NodeToBeDeleted = GetNode(Position, Node);
	if (NodeToBeDeleted)
	{
		if (NodeToBeDeleted->m_Children != 0 && !IsUpwardsRecursive)
		{
			// We are not a leaf node and we must therefore continue the search
			// in this nodes m_Children
			uint32_t Octant = NodeToBeDeleted->GetOctantForPosition(Position);

			// If it has a node in the octant we are looking for try removing
			// that one instead. Otherwise, the n
			if ((Node->m_Children >> Octant) & 1)
			{
				RemoveNode(Position, Nodes.at((Node->m_Location << 3) | Octant));
			}
		}
		else if (NodeToBeDeleted->m_Location != 1) // We are not the RootNode
		{
			uint32_t NodeOctant = NodeToBeDeleted->m_Location & 7;
			OctreeNode<T>* ParentNode = Nodes[(NodeToBeDeleted->m_Location >> 3)];
			ParentNode->m_Children ^= (1 << NodeOctant); // Remove from parents m_Children

			//RemoveNode(AddData, RemoveData, ParentNode->m_LocalPosition, ParentNode, true);

			Nodes.erase(NodeToBeDeleted->m_Location);
			//TODO: Check if all the sister nodes are empty - if so, remove the parent node aswell
			delete NodeToBeDeleted;
		}
	}
}
