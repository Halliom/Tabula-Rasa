#pragma once

#include <deque>

#include "ThreadSystem.h"
#include "CriticalSection.h"
#include "../Console.h"

typedef std::deque<AsyncJob*> JobQueue;

class Thread
{
public:
	Thread();

	~Thread();

	void Run();

	void QueueJob(AsyncJob* Job);

	AsyncJob* StealJob();

	int GetJobCount();

	int				m_ThreadID;
	ThreadHandle	m_Handle;
	JobQueue		m_Queue;
	CriticalSection m_CriticalSection;
};