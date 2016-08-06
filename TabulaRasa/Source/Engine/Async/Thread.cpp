#include "Thread.h"

Thread::Thread() :
	m_ThreadID(0)
{
}

Thread::~Thread()
{
	ThreadSystem::RemoveThread();
}

void Thread::Run()
{
	while (true)
	{
		if (GetJobCount() > 0)
		{
			// Get the job at the back
			AsyncJob* Job = m_Queue.back();
			m_Queue.pop_back();

			Job->Execute();
		}
		else
		{
			Thread* OtherThread = ThreadSystem::GetNonEmptyThread();

			if (OtherThread)
				OtherThread->StealJob();
		}
	}
}

void Thread::QueueJob(AsyncJob* Job)
{
	assert(Job != NULL);

	// Push it to the back since the Run function processes these
	// in order from back to front so it becomes a LIFO queue
	m_Queue.push_back(Job);
}

AsyncJob* Thread::StealJob()
{
	SCOPED_CS(m_CriticalSection);
	return m_Queue.back();
}

int Thread::GetJobCount()
{
	SCOPED_CS(m_CriticalSection);
	return m_Queue.size();
}
