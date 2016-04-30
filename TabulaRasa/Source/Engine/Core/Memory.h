#pragma once

#include <assert.h>
#include <cstdint>

#include "../Chunk.h"

__forceinline unsigned char* AlignAddress(unsigned char* Address, size_t Alignment)
{
	return (unsigned char*) ((reinterpret_cast<unsigned char>(Address) + static_cast<unsigned char>(Alignment - 1)) & static_cast<unsigned char>(~(Alignment - 1)));
}

__forceinline size_t CalcualteAlignmentAdjustment(const unsigned char* Address, size_t Alignment)
{
	size_t Adjustment = Alignment - (reinterpret_cast<unsigned char>(Address) & static_cast<unsigned char>(Alignment - 1));

	if (Adjustment == Alignment)
		return 0;

	return Adjustment;
}

__forceinline uint8_t AlignForwardAdjustmentWithHeader(const unsigned char* Address, uint8_t Alignment, uint8_t HeaderSize)
{
	uint8_t Adjustment = CalcualteAlignmentAdjustment(Address, Alignment);

	uint8_t NeededSpace = HeaderSize;

	if (Adjustment < NeededSpace)
	{
		NeededSpace -= Adjustment;

		//Increase adjustment to fit header
		Adjustment += Alignment * (NeededSpace / Alignment);

		if (NeededSpace % Alignment > 0)
			Adjustment += Alignment;
	}

	return Adjustment;
}

struct LinearAllocator
{
	LinearAllocator(unsigned char* StartAddress, size_t MaxByteSize);

	~LinearAllocator();

	unsigned char* Allocate(size_t NumBytes, size_t ByteAlignment);

	void ClearMemory();

	unsigned char* m_pStartAddress;
	size_t m_MaxByteSize;

	unsigned char* m_pCurrentPos;

	size_t m_UsedMemory; /* Total amount of bytes used */
	size_t m_NumAllocations; /* Total number of allocations made */
};

struct FreeList
{
	FreeList(unsigned char* StartAddress, size_t MaxByteSize);
	~FreeList();

	unsigned char* Allocate(size_t Size, size_t Alignment);
	
	void Free(unsigned char* Pointer);

	struct AllocationHeader
	{
		size_t	m_Size;
		uint8_t	m_AlignAdjustment;
	};

	struct MemorySlot
	{
		size_t		m_Size;
		MemorySlot* m_pNext;
	};

	unsigned char* m_pStartAddress;
	size_t m_MaxByteSize;

#ifdef _DEBUG
	size_t m_UsedMemory; /* Total amount of bytes used */
	size_t m_NumAllocations; /* Total number of allocations made */
#endif

	MemorySlot* m_pNextFree;
};

template<typename T>
struct MemoryPool
{
	MemoryPool(unsigned char* StartAddress, size_t MaxByteSize);

	~MemoryPool();

	T* Allocate();
	T* AllocateNew();

	void Deallocate(T* Pointer);
	void DeallocateDelete(T* Pointer);

	struct MemorySlot
	{
		T			m_Value; /* The value for the slot, or null if free */
		MemorySlot* m_pNext; /* Pointer to the next free slot of memory */
	};

	unsigned char* m_pStartAddress; /* The start address for the memory block */
	size_t m_MaxByteSize; /* Maximum number of bytes this memory block gets to use, marks the end the block*/

#ifdef _DEBUG
	size_t m_UsedMemory; /* Total amount of bytes used */
	size_t m_NumAllocations; /* Total number of allocations made */
#endif

	size_t m_ObjectSize; /* The computed size of the object + pointer to next free object */
	size_t m_Alignment; /* The alignment for the memoryslot*/

	MemorySlot* m_pFreeList; /* Singly-linked list of all free memory chunks with the head first */
};

template<typename T>
MemoryPool<T>::MemoryPool(unsigned char* StartAddress, size_t MaxByteSize) :
	m_pStartAddress(StartAddress),
	m_MaxByteSize(MaxByteSize)
{
#ifdef _DEBUG
	m_UsedMemory = 0;
	m_NumAllocations = 0;
#endif

	m_ObjectSize = sizeof(MemorySlot);
	m_Alignment = __alignof(MemorySlot);
	
	size_t Adjustment = CalcualteAlignmentAdjustment(StartAddress, __alignof(MemorySlot));
	
	m_pFreeList = (MemorySlot*) (StartAddress + Adjustment);

	size_t NumObjects = (MaxByteSize - Adjustment) / m_ObjectSize;
	size_t LastObject = NumObjects - 1;
	for (unsigned int i = 0; i < NumObjects; ++i)
	{
		m_pFreeList[i].m_pNext = &m_pFreeList[i + 1];
	}
	m_pFreeList[LastObject].m_pNext = NULL;
}

template<typename T>
MemoryPool<T>::~MemoryPool()
{
#ifdef _DEBUG
	// If m_NumAllocations != 0 then we have unallocated memory which will leak
	assert(m_NumAllocations == 0 && m_UsedMemory == 0);
#endif

	m_pStartAddress = 0;
	m_MaxByteSize = 0;
	m_pFreeList = NULL;
}

template<typename T>
__forceinline T* MemoryPool<T>::Allocate()
{
	if (m_pFreeList == NULL)
	{
		return NULL;
	}
	else
	{
		T* Result = &m_pFreeList->m_Value;
		m_pFreeList = m_pFreeList->m_pNext;

#ifdef _DEBUG
		++m_NumAllocations;
		m_UsedMemory += m_ObjectSize;
#endif

		return Result;
	}
}

template<typename T>
__forceinline T* MemoryPool<T>::AllocateNew()
{
	if (m_pFreeList->m_pNext == NULL)
	{
		return NULL;
	}
	else
	{
		T* Result = new (&m_pFreeList->m_Value) T;
		m_pFreeList = m_pFreeList->m_pNext;

#ifdef _DEBUG
		++m_NumAllocations;
		m_UsedMemory += m_ObjectSize;
#endif

		return Result;
	}
}

template<typename T>
__forceinline void MemoryPool<T>::Deallocate(T* Pointer) // TODO: Add __forceinline
{
	MemorySlot* Slot = (MemorySlot*) Pointer;
	
	Slot->m_pNext = m_pFreeList;
	m_pFreeList = Slot;

#ifdef _DEBUG
	--m_NumAllocations;
	m_UsedMemory -= m_ObjectSize;
#endif
}

template<typename T>
__forceinline void MemoryPool<T>::DeallocateDelete(T* Pointer)
{
	MemorySlot* Slot = (MemorySlot*) Pointer;
	Pointer->~T();

	Slot->m_pNext = m_pFreeList->m_pNext;
	m_pFreeList = Slot;

#ifdef _DEBUG
	--m_NumAllocations;
	m_UsedMemory -= m_ObjectSize;
#endif
}

template<typename T>
__forceinline unsigned char* Allocate(LinearAllocator* Allocator, size_t NumInstances = 1)
{
	return Allocator->Allocate(sizeof(T) * NumInstances, __alignof(T));
}

template<typename T>
__forceinline T* AllocateWithType(LinearAllocator* Allocator, size_t NumInstances = 1)
{
	return (T*) Allocator->Allocate(sizeof(T) * NumInstances, __alignof(T));
}

#define GB(b) MB(b) * 1000
#define MB(b) KB(b) * 1000
#define KB(b) b * 1000

class GameMemoryManager
{
public:

	GameMemoryManager();
	~GameMemoryManager();

	bool Initialize();

	void ClearTransientMemory();

	LinearAllocator*				m_pTransientFrameMemory;
	LinearAllocator*				m_pRenderingMemory;
	MemoryPool<Chunk>*		m_pChunkAllocator;

private:

	unsigned char* m_pGameMemory;

};

extern GameMemoryManager* g_MemoryManager;

template<typename T>
__forceinline T* AllocateTransient(size_t Num)
{
	return (T*) g_MemoryManager->m_pTransientFrameMemory->Allocate(sizeof(T) * Num, __alignof(T));
}
