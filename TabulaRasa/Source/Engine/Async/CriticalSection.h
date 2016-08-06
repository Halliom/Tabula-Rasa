#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

class CriticalSection
{
public:

	CriticalSection()
	{
		InitializeCriticalSection(&m_CriticalSection);
	}

	~CriticalSection()
	{
		DeleteCriticalSection(&m_CriticalSection);
	}

	void Enter()
	{
		EnterCriticalSection(&m_CriticalSection);
	}

	void Leave()
	{
		LeaveCriticalSection(&m_CriticalSection);
	}

private:

	CRITICAL_SECTION m_CriticalSection;
};

class ScopedCriticalSection
{
public:

	ScopedCriticalSection(const CriticalSection& CriticalSection) :
		m_CriticalSection(CriticalSection)
	{
		m_CriticalSection.Enter();
	}

	~ScopedCriticalSection()
	{
		m_CriticalSection.Leave();
	}

private:

	CriticalSection m_CriticalSection;
};

#define SCOPED_CS(cs) ScopedCriticalSection _ScopedCriticalSection(cs)