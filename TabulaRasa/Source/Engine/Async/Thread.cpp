#include "Thread.h"

#include "Job.h"

Thread::Thread() :
	m_ThreadID(0)
{
}

Thread::~Thread()
{
	ThreadSystem::RemoveThread(this);
}

void Thread::Run()
{
	while (true)
    {
		IJob* NextJob = ThreadSystem::GetNextJob();
		if (NextJob)
		{
			NextJob->Execute();
		}
	}
}
