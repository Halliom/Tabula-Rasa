#pragma once

class VoxelDictionary
{
public:

    void Add(unsigned int Key, class OctreeNode* Data);

    void Init(unsigned int Size);

    void Destroy();

    unsigned int m_Count;

    unsigned int m_Size;

    class OctreeNode** m_Data;

};
