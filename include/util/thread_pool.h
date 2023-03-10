#pragma once

#include <vector>
#include <thread>
#include <functional>
#include "util/tsque.h"

struct thread_pool
{
private:
	struct work_item
	{
		bool poison;
		std::function<void()> work;
	};

	std::vector<std::thread> m_threads;
	tsque<work_item> m_work; // if the thread should stop, work to be done

public:
	thread_pool(int thread_count)
	{
		m_threads.reserve(thread_count);

		for (int i = 0; i < thread_count; i++)
		{
			m_threads.push_back(std::thread([&]() 
				{
					while (true)
					{
						work_item work = m_work.pop_front();
						if (work.poison) break;
						work.work();
					}
				}
			));
		}
	}

	~thread_pool()
	{
		shutdown();
	}

	// joins all threads
	// allows work to complete
	void shutdown()
	{
		for (int i = 0; i < m_threads.size(); i++)
		{
			m_work.push_back(work_item{ true, {} }); // option: push_front would stop all queued work
		}

		for (int i = 0; i < m_threads.size(); i++)
		{
			if (m_threads.at(i).joinable())
			{
				m_threads.at(i).join();
			}
		}
	}

	void thread(const std::function<void()>& work)
	{
		m_work.push_back(work_item{ false, work });
	}
};