#include "Memory.h"

#include <Windows.h>

#include "../Chunk.h"

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

#ifdef _DEBUG
	m_NumAllocations = 0;
	m_UsedMemory = 0;
#endif
}

FreeList::~FreeList()
{
#ifdef _DEBUG
	// If m_NumAllocations != 0 then we have unallocated memory which will leak
	assert(m_NumAllocations == 0 && m_UsedMemory == 0);
#endif
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
		uint8_t Adjustment = AlignForwardAdjustmentWithHeader((unsigned char*)Current, Alignment, sizeof(AllocationHeader));
		size_t TotalSize = Size + Adjustment;
		if (Current->m_Size < TotalSize) // No space here, move on:(
		{
			// Move along
			Previous = Current;
			Current = Current->m_pNext;
			continue;
		}

		if (Current->m_Size - TotalSize <= sizeof(AllocationHeader)) // We can't fit an allocation here
		{
			// Increase the size
			TotalSize = Current->m_Size;
			if (Previous != NULL)
			{
				Previous->m_pNext = Current->m_pNext;
			}
			else
			{
				Current = Current->m_pNext;
			}
		}
		else
		{
			// Create a new MemorySlot containing the new free partition that sits right after 
			// the current allocation
			MemorySlot* Next = (MemorySlot*)(((unsigned char*)Current) + TotalSize);
			Next->m_Size = Current->m_Size - TotalSize;
			Next->m_pNext = Current->m_pNext;

			if (Previous != NULL)
				Previous->m_pNext = Next;
			else
				m_pNextFree = Next;
		}

		unsigned char* Result = ((unsigned char*)Current) + Adjustment;
		AllocationHeader* AllocHeader = (AllocationHeader*)(Result - sizeof(AllocationHeader));
		AllocHeader->m_Size = TotalSize;
		AllocHeader->m_AlignAdjustment = Adjustment;

#ifdef _DEBUG
		++m_NumAllocations;
		m_UsedMemory += TotalSize;
#endif

		return Result;
	}
	return NULL;
}

void FreeList::Free(void* VoidPointer)
{
	unsigned char* Pointer = (unsigned char*) VoidPointer;
	AllocationHeader* AllocHeader = (AllocationHeader*) (Pointer - sizeof(AllocationHeader));
	
	size_t SlotSize = AllocHeader->m_Size;
	unsigned char* MemorySlotBegin = Pointer - AllocHeader->m_AlignAdjustment;
	unsigned char* MemorySlotEnd = MemorySlotBegin + SlotSize;

	MemorySlot* Previous = NULL;
	MemorySlot* Current = m_pNextFree;

	while (Current != NULL)
	{
		// The current memory slot is _after_ the current slot
		if (((unsigned char*) Current) >= MemorySlotEnd)
		{
			break;
		}

		Previous = Current;
		Current = Current->m_pNext;
	}

	if (Previous == NULL)
	{
		Previous = (MemorySlot*) MemorySlotBegin;
		Previous->m_Size = SlotSize;
		Previous->m_pNext = m_pNextFree;

		m_pNextFree = Previous;
	}
	else if (((unsigned char*) Previous) + Previous->m_Size == MemorySlotBegin) // We found the slot just _before_ the deallocated memory
	{
		Previous->m_Size += SlotSize; // Just add this size as well
	}
	else
	{
		// Insert new free slot
		MemorySlot* Next = (MemorySlot*) MemorySlotBegin;
		Next->m_Size = SlotSize;
		Next->m_pNext = Previous->m_pNext;
		Previous->m_pNext = Next;
		Previous = Next;
	}

	// If Current is the slot just _after_ the deallocated memory simply extend
	// the previous one (which is going to be "this" one) (see above)
	if (Current != NULL && ((unsigned char*) Current) == MemorySlotEnd)
	{
		Previous->m_Size += Current->m_Size;
		Previous->m_pNext = Current->m_pNext;
	}

#ifdef _DEBUG
	--m_NumAllocations;
	m_UsedMemory -= SlotSize;
#endif
}

GameMemoryManager::GameMemoryManager()
{
	m_pMemoryBuffer = NULL;
}

GameMemoryManager::~GameMemoryManager()
{
	delete[] m_pMemoryBuffer;
}

bool GameMemoryManager::Initialize()
{
	m_pMemoryBuffer = new unsigned char[MB(128)];

	if (m_pMemoryBuffer == NULL)
		return false;

	m_pTransientFrameMemory = new LinearAllocator(m_pMemoryBuffer, MB(8)); // 8 MB total
	m_pRenderingMemory = new LinearAllocator(m_pMemoryBuffer + MB(8), MB(12)); // 20 MB total
	m_pChunkAllocator = new MemoryPool<Chunk>(m_pMemoryBuffer + MB(20), MB(80)); // 100 MB total
	m_pGameMemory = new FreeList(m_pMemoryBuffer + MB(100), MB(28)); // 128 MB total

	return true;
}

void GameMemoryManager::ClearTransientMemory()
{
	m_pTransientFrameMemory->ClearMemory();
}
