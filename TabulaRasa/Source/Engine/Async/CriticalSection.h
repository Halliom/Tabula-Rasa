#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

#include "Async.h"

class TicketMutex
{
public:

	TicketMutex()
	{
        m_Ticket = m_NowServing = 0;
	}
    
	~TicketMutex()
	{
	}

    void Enter()
	{
        int Ticket = Async::Increment(&m_Ticket);
        while (m_NowServing != Ticket);
	}

	void Leave()
	{
        ++m_NowServing;
	}

private:

    int volatile m_Ticket;
    int volatile m_NowServing;
};

class ScopedMutex
{
public:

	ScopedMutex(const TicketMutex& Mutex) :
		m_Mutex(Mutex)
	{
		m_Mutex.Enter();
	}

	~ScopedCriticalSection()
	{
		m_Mutex.Leave();
	}

private:

	TicketMutex m_Mutex;
};

#define SCOPED_MUTEX(mutex) ScopedMute _ScopedMutexLock(mutex)
