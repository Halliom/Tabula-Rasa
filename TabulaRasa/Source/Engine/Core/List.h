#pragma once

#include <cstdint>
#include "Memory.h"

template <typename T>
class List
{
public:

	List(FreeList* Memory);
	
	List();

	~List();

	void Reserve(int NumElements);

	int Push(const T& Element);

	T& PushEmpty();

	T Pop();

	T& operator[](int Index);

	int IndexOf(const T& Element);

	void Remove(int Index);

	void Remove(const T& Element);

	T* Data()
	{
		return m_pBuffer;
	}

	/**
	 * Number of elements in the list
	 */
	size_t Size;

private:

	FreeList* m_pAllocator;

	T* m_pBuffer;

	size_t m_BytesUsed;

	size_t m_BytesAllocated;

};

template<typename T>
List<T>::List(FreeList* Memory) : 
	m_pAllocator(Memory),
	Size(0),
	m_pBuffer(NULL),
	m_BytesUsed(0),
	m_BytesAllocated(0)
{
}

template<typename T>
List<T>::List()	: 
	Size(0),
	m_pBuffer(NULL),
	m_BytesUsed(0),
	m_BytesAllocated(0)
{
	if (g_MemoryManager)
		m_pAllocator = g_MemoryManager->m_pGameMemory;
}

template<typename T>
inline List<T>::~List()
{
	if (m_pBuffer != NULL)
	{
		m_pAllocator->Free(m_pBuffer);
		m_pBuffer = NULL;
		Size = 0;
		m_BytesUsed = 0;
		m_BytesAllocated = 0;
	}
}

template<typename T>
void List<T>::Reserve(int NumElements)
{
	size_t NewSize = NumElements * sizeof(T);
	T* OldBuffer = m_pBuffer;
	if (NewSize == 0)
	{
		m_pBuffer = NULL;
	}
	else
	{
		m_pBuffer = (T*) m_pAllocator->Allocate(NewSize, __alignof(T));
		memset(m_pBuffer, 0, NewSize);
	}

	// If we wan't to expand the list, copy the number of bytes used and the rest will
	// be blank. If we want to shrink the list then we can only copy as many bytes as we
	// currently use, otherwise we would go "out of bounds"
	int CopyBytes = NumElements <= Size ? NewSize : m_BytesUsed;
	memcpy(m_pBuffer, OldBuffer, CopyBytes);
	if (OldBuffer)
	{
		m_pAllocator->Free(OldBuffer);
	}

	m_BytesAllocated = NewSize;
	m_BytesUsed = CopyBytes;
	if (NumElements <= Size) // If we're shrinking the size of the list
	{
		Size = NumElements;
	}
}

template<typename T>
int List<T>::Push(const T& Element)
{
	size_t NewSize = m_BytesUsed + sizeof(T);
	if (NewSize > m_BytesAllocated)
	{
		// Save a pointer to the old buffer and allocate new byffer
		T* OldBuffer = m_pBuffer;
		m_pBuffer = (T*) m_pAllocator->Allocate(NewSize, __alignof(T));

		// Copy over the bytes from the old buffer to the new one and
		// free the old buffer
		memcpy(m_pBuffer, OldBuffer, m_BytesUsed);
		if (OldBuffer)
		{
			m_pAllocator->Free(OldBuffer);
		}
		m_BytesAllocated = NewSize;
	}
	
	m_pBuffer[Size] = Element;
	m_BytesUsed = NewSize;

	return Size++;
}

template<typename T>
inline T& List<T>::PushEmpty()
{
	size_t NewSize = m_BytesUsed + sizeof(T);
	if (NewSize > m_BytesAllocated)
	{
		// Save a pointer to the old buffer and allocate new byffer
		T* OldBuffer = m_pBuffer;
		m_pBuffer = (T*)m_pAllocator->Allocate(NewSize, __alignof(T));

		// Copy over the bytes from the old buffer to the new one and
		// free the old buffer
		memcpy(m_pBuffer, OldBuffer, m_BytesUsed);
		if (OldBuffer)
		{
			m_pAllocator->Free(OldBuffer);
		}
		m_BytesAllocated = NewSize;
	}

	m_BytesUsed = NewSize;

	return *(m_pBuffer + Size++);
}

template<typename T>
T List<T>::Pop()
{
	m_BytesUsed -= sizeof(T);
	return m_pBuffer[--Size];
}

template<typename T>
T& List<T>::operator[](int Index)
{
	if (Index >= Size)
	{
		//assert(false);
		Index = 0;
	}
	return m_pBuffer[Index];
}

template<typename T>
int List<T>::IndexOf(const T& Element)
{
	for (int i = 0; i < Size; ++i)
	{
		if (memcmp(&m_pBuffer[i], &Element, sizeof(T)) == 0)
		{
			return i;
		}
	}
	return -1;
}

template<typename T>
void List<T>::Remove(int Index)
{
	assert(Index >= 0 && Index < Size);

	int HigerSideElements = Size - Index - 1;

	memcpy(
		m_pBuffer + Index, 
		m_pBuffer + Index + 1, 
		HigerSideElements * sizeof(T));

	--Size;
	m_BytesUsed -= sizeof(T);
}

template<typename T>
void List<T>::Remove(const T& Element)
{
	int Index = IndexOf(Element);
	Remove(Index);
}
