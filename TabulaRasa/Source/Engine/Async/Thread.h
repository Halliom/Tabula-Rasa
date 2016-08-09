#pragma once

#include "ThreadSystem.h"
#include "CriticalSection.h"

class Thread
{
public:
	Thread();

	~Thread();

	void Run();

	int				m_ThreadID;
	ThreadHandle	m_Handle;
	CriticalSection m_CriticalSection;
};