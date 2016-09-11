#pragma once

#include "ThreadSystem.h"
#include "Mutex.h"

class Thread
{
public:
	Thread();

	~Thread();

	void Run();

	int				m_ThreadID;
	ThreadHandle	m_Handle;
	TicketMutex		m_TicketMutex;
};