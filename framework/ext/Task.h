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
	TaskPool()
		: m_pool (12) // should either allow user to specify or make as many cores as the processor has, an SDL function might do this?
	{}

	// execute work a single time on another thread
	void Thread(const std::function<void()>& work)
	{
		m_pool.thread(work);
	}

	// execute work on the main thread until work returns true
	// each call to TickCoroutines will call work once
	void Coroutine(const std::function<bool()>& work)
	{
		std::unique_lock lock(m_coworkMutex);
		return m_cowork.push_back(work);
	}

	// execute work on the main thread at the end of the frame, but before events fire
	// runs one time
	void Defer(const std::function<void()>& work)
	{
		return Coroutine([work]() { work(); return true; });
	}

	void TickCoroutines()
	{
		std::unique_lock lock(m_coworkMutex);
		for (int i = 0; i < m_cowork.size(); i++)
		{
			if (m_cowork.at(i)())
			{
				m_cowork.at(i) = m_cowork.back();
				m_cowork.pop_back();
				i--;
			}
		}
	}

	void ShutdownAndWait()
	{
		m_pool.shutdown();
	}
};

struct TaskSyncPoint
{
private:
	std::mutex m_mutex;
	std::condition_variable m_cond;
	
	int m_value; // could template
	
public:
	TaskSyncPoint(
		int value
	)
		: m_value (value)
	{}

	void Tick()
	{
		std::unique_lock lock(m_mutex);
		m_value -= 1;
		m_cond.notify_one();
	}

	void BlockUntilZero()
	{
		std::unique_lock lock(m_mutex);
		m_cond.wait(lock, [&]() { return m_value == 0; });
	}
};