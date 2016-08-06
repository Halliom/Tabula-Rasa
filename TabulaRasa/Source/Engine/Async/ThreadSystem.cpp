#include "ThreadSystem.h"

#include "Thread.h"

std::vector<Thread*> ThreadSystem::g_ThreadPool;

#ifdef _WIN32
DWORD WINAPI ThreadProc(LPVOID Param)
#else
void ThreadProc(void* Param)
#endif
{
	// Get the Thread from the argument and set its ID
	Thread* ThreadToLaunch = (Thread*)Param;
	ThreadToLaunch->m_ThreadID = ThreadSystem::GetCurrentThreadID();

	// Add the thread to the list
	ThreadSystem::g_ThreadPool.push_back(ThreadToLaunch);

	// Run the thread
	ThreadToLaunch->Run();

#ifdef _WIN32
	return TRUE;
#else
	return
#endif
}

ThreadHandle ThreadSystem::LaunchThread(const Thread& ThreadToLaunch)
{
#ifdef _WIN32
	DWORD ThreadID = 0;
	ThreadHandle Handle = CreateThread(
		NULL,
		0,
		ThreadProc,
		(void*)(&ThreadToLaunch),
		0,
		&ThreadID);
#else
	ThreadHandle Handle = pthread_create();
#endif

	return Handle;
}

void ThreadSystem::RemoveThread()
{
#ifdef _WIN32
	CloseHandle(GetCurrentThread()->m_Handle);
#else
	// pthread something
#endif
}

int ThreadSystem::GetCurrentThreadID()
{
#ifdef _WIN32
	DWORD ThreadID = GetCurrentThreadId();
#else
	/*pthread_id_np_t   tid;
	tid = pthread_getthreadid_np();*/
#endif
	return ThreadID;
}

Thread* ThreadSystem::GetCurrentThread()
{
	int CurrentThreadID = GetCurrentThreadID();
	for (auto& Thread : g_ThreadPool)
	{
		if (Thread->m_ThreadID == CurrentThreadID)
			return Thread;
	}
	return NULL;
}

void ThreadSystem::AddJob(AsyncJob* Job)
{
	GetCurrentThread()->QueueJob(Job);
}

void ThreadSystem::AddJobs(AsyncJob* Jobs, size_t NumJobs)
{
	size_t NumThreads = g_ThreadPool.size();
	for (size_t i = 0; i < NumJobs; ++i)
	{
		g_ThreadPool[i % NumThreads]->QueueJob(Jobs + i);
	}
}

Thread* ThreadSystem::GetNonEmptyThread()
{
	for (auto& Thread : g_ThreadPool)
	{
		if (Thread->GetJobCount() > 0)
			return Thread;
	}
	return NULL;
}
