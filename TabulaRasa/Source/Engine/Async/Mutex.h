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
		m_Ticket = new unsigned int;
		m_NowServing = new unsigned int;
		*m_Ticket = 0;
		*m_NowServing = 1;
	}
    
	~TicketMutex()
	{
		delete m_Ticket;
		delete m_NowServing;
	}

	void Enter()
	{
        int Ticket = Async::Increment(m_Ticket);
        while ((*m_NowServing) != Ticket);
	}

	void Leave()
	{
		Async::Increment(m_NowServing);
	}

private:

    volatile unsigned int* m_Ticket;
    volatile unsigned int* m_NowServing;
};

class ScopedMutex
{
public:

	ScopedMutex(TicketMutex* Mutex) :
		m_Mutex(Mutex)
	{
		m_Mutex->Enter();
	}

	~ScopedMutex()
	{
		m_Mutex->Leave();
	}

private:

	TicketMutex* m_Mutex;
};

#define SCOPED_LOCK(mutex) ScopedMutex _ScopedMutexLock(&mutex)
