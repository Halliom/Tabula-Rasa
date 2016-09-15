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
#include "Job.h"

typedef std::deque<IJob*> JobQueue;

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

	static void			ScheduleJob(IJob* Job);
	static void			ScheduleJobs(IJob* Jobs, size_t NumJobs);
	static IJob*        GetNextJob();
	static void			Wait(IJob* Job);

public: // Private?

	static std::vector<Thread*> g_ThreadPool;
	static Thread*				g_MainThread;
	static JobQueue				g_JobQueue;
	static TicketMutex			g_TicketMutex;
};