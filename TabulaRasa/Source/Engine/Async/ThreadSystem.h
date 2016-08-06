#pragma once

#ifdef _WIN32
#include <Windows.h>
typedef HANDLE ThreadHandle;
#else
#include <pthread.h>
typedef pthread_t ThreadHandle;
#endif

#include <vector>

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

private:

	JobFunction m_Function;
	void*		m_pData;
	bool		m_bFinished;
};

class ThreadSystem
{
public:

	static ThreadHandle LaunchThread(const class Thread& ThreadToLaunch);
	static void RemoveThread();

	static int GetCurrentThreadID();
	static Thread* GetCurrentThread();

	static void AddJob(AsyncJob* Job);
	static void AddJobs(AsyncJob* Jobs, size_t NumJobs);

	static Thread* GetNonEmptyThread();

	static std::vector<Thread*> g_ThreadPool;
};