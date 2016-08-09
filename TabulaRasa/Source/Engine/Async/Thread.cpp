#include "Thread.h"

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
		AsyncJob* NextJob = ThreadSystem::GetNextJob();
		if (NextJob)
		{
			NextJob->Execute();
		}
	}
}
