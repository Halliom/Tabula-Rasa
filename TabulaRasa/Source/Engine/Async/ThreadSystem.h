#pragma once

#ifdef _WIN32
#include <Windows.h>
typedef HANDLE ThreadHandle;
#else
#include <pthread.h>
typedef pthread_t ThreadHandle;
#endif

#include <vector>
#include <deque>

#include "Mutex.h"

#define JOB_FUNCTION(FunctionName) void FunctionName(void* Data)

typedef void(*JobFunction)(void*);

class AsyncJob
{
public:
	AsyncJob(JobFunction JobFunc, void* Data) :
		m_Function(JobFunc),
		m_pData(Data),
		m_bFinished(false)
	{
	}

	void Execute()
	{
		// Call the function with the data
		(*m_Function)(m_pData);
		m_bFinished = true;
	}

	bool Finished()
	{
		return m_bFinished;
	}

private:

	JobFunction     m_Function;
	void*           m_pData;
	volatile bool	m_bFinished;
};

typedef std::deque<AsyncJob*> JobQueue;

class ThreadSystem
{
public:

	static void InitializeThreads(int NumWorkerThreads);
	static void DestroyThreads();

	static ThreadHandle LaunchThread(class Thread& ThreadToLaunch);
	static void			RemoveThread(Thread* ThreadToRemove);

	static ThreadHandle GetCurrentGameThreadHandle();
	static int			GetCurrentGameThreadID();
	static Thread*		GetCurrentGameThread();

	static void			ScheduleJob(AsyncJob* Job);
	static void			ScheduleJobs(AsyncJob* Jobs, size_t NumJobs);
	static AsyncJob*	GetNextJob();
	static void			Wait(AsyncJob* Job);

public: // Private?

	static std::vector<Thread*> g_ThreadPool;
	static Thread*				g_MainThread;
	static JobQueue				g_JobQueue;
	static TicketMutex			g_TicketMutex;
};