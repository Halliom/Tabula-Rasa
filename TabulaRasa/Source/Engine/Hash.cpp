#include "Hash.h"

#include "Octree.h"

void VoxelDictionary::Init(unsigned int Size)
{
    m_Size = Size;
    m_Data = new OctreeNode*[Size];
	memset(m_Data, NULL, m_Size * sizeof(OctreeNode*));
}

void VoxelDictionary::Destroy()
{
    delete[] m_Data;
}

void VoxelDictionary::Add(unsigned int Key, OctreeNode* Data)
{
    unsigned int Hash = Key * 2654435761 % 4294967296;

    unsigned int Index = Hash % m_Size;

    if (m_Data[Index] == NULL)
    {
        m_Data[Index] = Data;
    }
    else
    {
		// TODO: Collision
		return;
    }
}
