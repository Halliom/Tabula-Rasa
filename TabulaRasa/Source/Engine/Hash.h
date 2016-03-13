#pragma once

#include <stdint.h>

#define TABLE_SIZE 512

template<typename T>
class IntegerHashTable
{
public:

	IntegerHashTable();

	void Add(const uint32_t& Key, const T& Value);

	T* Get(const uint32_t& Key);

	void Remove(const uint32_t& Key);

private:

	unsigned int m_Size;
	unsigned int m_NumElements;
	
	T* m_pKeyValuePairs;

};

template<typename T>
inline IntegerHashTable<T>::IntegerHashTable()
{
	m_Size = TABLE_SIZE;
	m_NumElements = 0;

	m_pKeyValuePairs = new T[TABLE_SIZE];
	memset(m_pKeyValuePairs, NULL, sizeof(T) * TABLE_SIZE);
}

template<typename T>
inline void IntegerHashTable<T>::Add(const uint32_t& Key, const T& Value)
{
	// Knuths multiplicative method hash(i) = i * 2654435761 mod 2 ^ 32
	unsigned int Hash = (Key * 2654435761) % 4294967296;
	unsigned int Index = Hash % m_Size;

	if (m_pKeyValuePairs[Index])
	{
		return;
	}
	else
	{
		m_pKeyValuePairs[Index] = Value;
	}
}

template<typename T>
inline T* IntegerHashTable<T>::Get(const uint32_t& Key)
{
	unsigned int Hash = (Key * 2654435761) % 4294967296;
	unsigned int Index = Hash % m_Size;

	return m_pKeyValuePairs[Index];
}

template<typename T>
inline void IntegerHashTable<T>::Remove(const uint32_t& Key)
{
	unsigned int Hash = (Key * 2654435761) % 4294967296;
	unsigned int Index = Hash % m_Size;

	T* Value = m_pKeyValuePairs[Index];
	m_pKeyValuePairs[Index] = NULL;
	return Value;
}

