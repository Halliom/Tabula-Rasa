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
	if (m_pNextFree == NULL)
		return NULL;

	MemorySlot* Previous = NULL;
	MemorySlot* Current = m_pNextFree;

	while (Current != NULL)
	{
		if (Current->m_Size >= Size)
		{
			if (Current->m_Size - Size < sizeof(MemorySlot)) // If we can't partition
			{
				unsigned char* Result = (unsigned char*) Current;
				if (Previous)
				{
					Previous->m_pNext = Current->m_pNext;
					Current->m_pNext = NULL;
				}
				else
				{
					m_pNextFree = Current->m_pNext;
					Current->m_pNext = NULL;
				}
				return Result;
			}
			else
			{
				size_t InitialSize = Current->m_Size;
				unsigned char* Result = (unsigned char*) Current;

				MemorySlot* New = (MemorySlot*) (Result + Size);
				New->m_pNext = Current->m_pNext;
				if (Previous)
				{
					Previous->m_pNext = New;
				}
				else
				{
					m_pNextFree = New;
				}

				New->m_Size = InitialSize - Size;
				return Result;
			}
		}
		else
		{
			Previous = Current;
			Current = Current->m_pNext;
		}
	}
	return NULL;
}

void FreeList::MergeBlocks(MemorySlot* First, MemorySlot* Second)
{
	size_t TotalSize = First->m_Size + Second->m_Size;
	First->m_Size = TotalSize;
	if (Second->m_pNext == First) // They are in reverse order
	{
		// Do nothing since this one is already pointing to the next element
	}
	else
	{
		First->m_pNext = Second->m_pNext;
	}
}

void FreeList::Free(unsigned char* Pointer, size_t Size)
{
	// This frees this memory chunk
	MemorySlot* Current = (MemorySlot*) Pointer;
	Current->m_pNext = m_pNextFree;
	Current->m_Size = Size;
	m_pNextFree = Current;

	// Search through the linked list of MemorySlots for a free one that sits
	// right next to the one we just freed
	MemorySlot* Previous = NULL;
	MemorySlot* Next = m_pNextFree->m_pNext;

	unsigned char* MemoryBlockBegin = (unsigned char*) Current;
	unsigned char* MemoryBlockEnd = ((unsigned char*) Current) + Current->m_Size;

	while (Next != NULL)
	{
		bool NeedsReordering = false;
		unsigned char* NextMemoryBlockBegin = (unsigned char*) Next;
		unsigned char* NextMemoryBlockEnd = ((unsigned char*) Next) + Next->m_Size;
		if (MemoryBlockBegin == NextMemoryBlockEnd)
		{
			if (Current == m_pNextFree)
			{
				NeedsReordering = true;
			}
			MergeBlocks(Next, Current);
			Current = Next;

			if (NeedsReordering)
			{
				m_pNextFree = Next;
			}
			
			MemoryBlockBegin = (unsigned char*) Current;
			MemoryBlockEnd = ((unsigned char*) Current) + Current->m_Size;
		}
		else if (MemoryBlockEnd == NextMemoryBlockBegin)
		{
			MergeBlocks(Current, Next);
			unsigned char* MemoryBlockEnd = ((unsigned char*) Current) + Current->m_Size;
		}
		
		Previous = Next;
		Next = Next->m_pNext;
	}
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
