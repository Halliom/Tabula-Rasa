#include "Memory.h"

#include <Windows.h>

LinearAllocator::LinearAllocator(unsigned char* StartAddress, size_t MaxByteSize) :
	m_pStartAddress(StartAddress),
	m_MaxByteSize(MaxByteSize),
	m_pCurrentPos(StartAddress)
{
#ifdef _DEBUG
	m_UsedMemory = 0;
	m_NumAllocations = 0;
#endif
}

LinearAllocator::~LinearAllocator()
{
#ifdef _DEBUG
	// If m_NumAllocations != 0 then we have unallocated memory which will leak
	assert(m_NumAllocations == 0 && m_UsedMemory == 0);
#endif
	
	m_pStartAddress = 0;
	m_pCurrentPos = 0;
	m_MaxByteSize = 0;
}

unsigned char* LinearAllocator::Allocate(size_t NumBytes, size_t ByteAlignment)
{
	size_t Adjustment = CalcualteAlignmentAdjustment(m_pStartAddress, ByteAlignment);

	if (m_UsedMemory + Adjustment + NumBytes > m_MaxByteSize)
	{
		// No allocation, we are out of memory
		return NULL;
	}

	// The aligned address
	unsigned char* Result = m_pCurrentPos + Adjustment;

	// Advance the current position to point to _after_ the latest allocation
	m_pCurrentPos = Result + NumBytes; 
	m_UsedMemory += Adjustment + NumBytes;
	++m_NumAllocations;

	return Result;
}

void LinearAllocator::ClearMemory()
{
	// Resets everything
	m_NumAllocations = 0;
	m_UsedMemory = 0;
	m_pCurrentPos = m_pStartAddress;
}

FreeList::FreeList(unsigned char* StartAddress, size_t MaxByteSize)
{
	m_pNextFree = (MemorySlot*) StartAddress;
	m_pNextFree->m_Size = MaxByteSize;
	m_pNextFree->m_pNext = NULL;
}

FreeList::~FreeList()
{
	m_pNextFree = NULL;
}

unsigned char* FreeList::Allocate(size_t Size, size_t Alignment)
{
	return NULL;
}

void FreeList::Free(unsigned char * Pointer)
{
}

GameMemoryManager::GameMemoryManager()
{
	m_pGameMemory = NULL;
}

GameMemoryManager::~GameMemoryManager()
{
	free(m_pGameMemory);
}

bool GameMemoryManager::Initialize()
{
	m_pGameMemory = new unsigned char[MB(100)];

	if (m_pGameMemory == NULL)
		return false;

	m_pTransientFrameMemory = new LinearAllocator(m_pGameMemory, (MB(8))); // 8 MB total
	m_pRenderingMemory = new LinearAllocator(m_pGameMemory + (MB(8)), MB(12)); // 20 MB total
	m_pChunkAllocator = new MemoryPool<Chunk>(m_pGameMemory + (MB(20)), MB(80)); // 100 MB total

	return true;
}

void GameMemoryManager::ClearTransientMemory()
{
	m_pTransientFrameMemory->ClearMemory();
}
