#include "ThreadSystem.h"

#include <algorithm>
#include <cassert>

#include "Thread.h"

std::vector<Thread*>	ThreadSystem::g_ThreadPool;
Thread*					ThreadSystem::g_MainThread;
JobQueue				ThreadSystem::g_JobQueue;
TicketMutex             ThreadSystem::g_TicketMutex;

#ifdef _WIN32
DWORD WINAPI ThreadProc(LPVOID Param)
#else
void* ThreadProc(void* Param)
#endif
{
	// Get the Thread from the argument and set its ID
	Thread* ThisThread = (Thread*)Param;
	ThisThread->m_ThreadID = ThreadSystem::GetCurrentGameThreadID();

	// Add the thread to the list
	ThreadSystem::g_ThreadPool.push_back(ThisThread);

	// Run the thread
	ThisThread->Run();

#ifdef _WIN32
	return TRUE;
#else
    return NULL;
#endif
}

void ThreadSystem::InitializeThreads(int NumWorkerThreads)
{
	// Get a reference to the main thread
	Thread* MainThread = new Thread();
	MainThread->m_Handle = GetCurrentGameThreadHandle();
	MainThread->m_ThreadID = GetCurrentGameThreadID();
	g_MainThread = MainThread;

	// Spawn worker threads
	for (int i = 0; i < NumWorkerThreads; ++i)
	{
		Thread* WorkerThread = new Thread();
		LaunchThread(*WorkerThread);
	}
}

void ThreadSystem::DestroyThreads()
{
	for (int i = 0; i < g_ThreadPool.size(); ++i)
	{
		delete g_ThreadPool[i];
	}
	g_ThreadPool.clear();

	// TODO: What to do about the main thread?
}

ThreadHandle ThreadSystem::LaunchThread(Thread& ThreadToLaunch)
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
	ThreadToLaunch.m_Handle = Handle;
#else
    ThreadHandle Handle;
	pthread_create(&Handle, NULL, ThreadProc, &ThreadToLaunch);
#endif

	return Handle;
}

void ThreadSystem::RemoveThread(Thread* ThreadToRemove)
{
#ifdef _WIN32
	// Close the handle, no need to remove from the ThreadPool since this only gets called from
	// DestroyThreads which clears the ThreadPool
	CloseHandle(ThreadToRemove->m_Handle);
#else
	// pthread something
#endif
}

ThreadHandle ThreadSystem::GetCurrentGameThreadHandle()
{
#ifdef _WIN32
	ThreadHandle Handle = GetCurrentThread();
#else
    ThreadHandle Handle = pthread_self();
#endif
	return Handle;
}

int ThreadSystem::GetCurrentGameThreadID()
{
#ifdef _WIN32
	DWORD ThreadID = GetCurrentThreadId();
#else
    ThreadHandle ThisThread = pthread_self();
    int ThreadID = (int)((size_t)ThisThread);
#endif
	return ThreadID;
}

Thread* ThreadSystem::GetCurrentGameThread()
{
	int CurrentThreadID = GetCurrentGameThreadID();
	for (int i = 0; i < g_ThreadPool.size(); ++i)
	{
		if (g_ThreadPool[i]->m_ThreadID == CurrentThreadID)
			return g_ThreadPool[i];
	}
	return NULL;
}

void ThreadSystem::ScheduleJob(AsyncJob* Job)
{
	assert(Job != NULL);

	SCOPED_LOCK(g_TicketMutex);

	// Push it to the back since the Run function processes these
	// in order from back to front so it becomes a LIFO queue
	g_JobQueue.push_back(Job);
}

void ThreadSystem::ScheduleJobs(AsyncJob* Jobs, size_t NumJobs)
{
	assert(Jobs != NULL);

	SCOPED_LOCK(g_TicketMutex);

	// Push it to the back since the Run function processes these
	// in order from back to front so it becomes a LIFO queue
	for (int i = 0; i < NumJobs; ++i)
		g_JobQueue.push_back(Jobs + i);
}

AsyncJob* ThreadSystem::GetNextJob()
{
	SCOPED_LOCK(g_TicketMutex);

	if (g_JobQueue.size() > 0)
	{
		AsyncJob* NextJob = g_JobQueue.back();
		g_JobQueue.pop_back();
		return NextJob;
	}
	else
	{
		return NULL;
	}
}

void ThreadSystem::Wait(AsyncJob* Job)
{
	while (!Job->Finished())
	{
		AsyncJob* NextJob = GetNextJob();
		if (NextJob)
		{
			NextJob->Execute();
		}
	}
}
