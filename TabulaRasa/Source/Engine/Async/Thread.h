#pragma once

#include "ThreadSystem.h"
#include "Mutex.h"

/**
 * Simple worker thread implementation that looks
 * for new work to be done and if it can find some
 * it does the work
 */
class Thread
{
public:
    
	Thread();

	~Thread();

	void Run();

public:
    
	int				m_ThreadID;
	ThreadHandle	m_Handle;
	TicketMutex		m_TicketMutex;
};