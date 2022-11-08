#pragma once

#include "util/thread_pool.h"
#include "util/pool_allocator.h"

#include <condition_variable>
#include <mutex>

struct TaskPool
{
private:
	thread_pool m_pool;
	std::vector<std::function<bool()>> m_cowork;
	std::mutex m_coworkMutex;
	
public:
    TaskPool();

	// execute work a single time on another thread
    void Thread(const std::function<void()>& work);

	// execute work on the main thread until work returns true
	// each call to TickCoroutines will call work once
    void Coroutine(const std::function<bool()>& work);

	// execute work on the main thread at the end of the frame, but before events fire
	// runs one time
    void Defer(const std::function<void()>& work);

    void TickCoroutines();
    void ShutdownAndWait();
};

struct TaskSyncPoint
{
private:
	std::mutex m_mutex;
	std::condition_variable m_cond;
	
	int m_value; // could template
	
public:
	TaskSyncPoint(int value);

    void Tick();
    void BlockUntilZero();
};
