#pragma once

#include <assert.h>
#include <cstdint>

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

template<typename T>
struct MemoryPool
{
	MemoryPool(unsigned char* StartAddress, size_t MaxByteSize);

	~MemoryPool();

	//T* Allocate(size_t Size);
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
T* MemoryPool<T>::Allocate()
{
	if (m_pFreeList->m_pNext == NULL)
	{
		return NULL;
	}
	else
	{
		T* Result = &m_pFreeList->m_Value;
		m_pFreeList = m_pFreeList->m_pNext;
		return Result;
	}

#ifdef _DEBUG
	++m_NumAllocations;
	m_UsedMemory += m_ObjectSize;
#endif
}

template<typename T>
T* MemoryPool<T>::AllocateNew()
{
	if (m_pFreeList->m_pNext == NULL)
	{
		return NULL;
	}
	else
	{
		T* Result = new (&m_pFreeList->m_Value) T;
		m_pFreeList = m_pFreeList->m_pNext;
		return Result;
	}

#ifdef _DEBUG
	++m_NumAllocations;
	m_UsedMemory += m_ObjectSize;
#endif
}

template<typename T>
__forceinline void MemoryPool<T>::Deallocate(T* Pointer)
{
	MemorySlot* Slot = (MemorySlot*) Pointer;
	
	Slot->m_pNext = m_pFreeList->m_pNext;
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

class GameMemoryManager
{

};