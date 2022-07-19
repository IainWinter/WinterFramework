#pragma once

// new event system
// should focus on the order of function execution

// main goal:
// should allow for handlers to classes without an interface for every type of function
// and events arent an interface

#include "util/tsque.h"

#include <mutex>
#include <vector>
#include <stdint.h>
#include <functional>
#include <unordered_map>
#include <assert.h>

using hash_t = uint64_t;

struct event_type
{
	hash_t m_hash = 0;
	int m_size = 0;
	const char* m_name = nullptr;
};

template<typename _e>
event_type make_event()
{
	static event_type e;
	if (e.m_hash == 0)
	{
		e.m_hash = typeid(_e).hash_code();
		e.m_size = sizeof(_e);
		e.m_name = typeid(_e).name();
	}

	return e;
}

struct event_pipe
{
	void* m_destination = nullptr; // bound with type info in lambdas
	std::function<void(void*)> m_send;
	std::function<bool(void*)> m_only_if;

	void send(void* event)
	{
		if (!m_only_if || m_only_if(event))
		{
			m_send(event);
		}
	}
};

template<typename _e>
struct event_pipe_wrapper
{
	event_pipe& m_pipe;

	event_pipe_wrapper(event_pipe& pipe) : m_pipe(pipe) {}

	void only_if(std::function<bool(const _e&)> func)
	{
		m_pipe.m_only_if = [func](void* event)
		{
			return func(*(_e*)event);
		};
	}
};

template<typename _e, typename _h>
event_pipe make_pipe(_h* handler)
{
	//static_assert(decltype(_h::on(_t&)));

	event_pipe pipe;
	pipe.m_destination = handler;
	pipe.m_send = [=](void* event)
	{
		handler->on(*(_e*)event);
	};

	return pipe;
}

struct event_sink
{
	event_type m_type;
	std::vector<event_pipe> m_pipes;

	template<typename _e, typename _h>
	event_pipe& add_pipe(_h* handler)
	{
		return m_pipes.emplace_back(make_pipe<_e, _h>(handler));
	}

	void remove_pipe(void* handler)
	{
		for (int i = 0; i < m_pipes.size(); i++)
		{
			if (m_pipes.at(i).m_destination == handler)
			{
				m_pipes.erase(m_pipes.begin() + i);
				--i; // dont break could have multiple pipes to same instance
			}
		}
	}

	void send(void* event)
	{
		for (event_pipe& pipe : m_pipes)
		{
			pipe.send(event);
		}
	}
};

struct event_manager
{
	std::unordered_map<hash_t, event_sink> m_sinks;
	std::vector<event_manager*> m_children; // doesnt own

	// needs to be template

	template<typename _e, typename _h>
	event_pipe_wrapper<_e> attach(_h* handler_ptr)
	{
		event_sink& sink = m_sinks[make_event<_e>().m_hash];
		event_pipe& pipe = sink.add_pipe<_e, _h>(handler_ptr);
		return event_pipe_wrapper<_e>(pipe);
	}

	void detach(void* handler)
	{
		for (auto itr = m_sinks.begin(); itr != m_sinks.end();)
		{
			itr->second.remove_pipe(handler);

			if (itr->second.m_pipes.size() == 0)
			{
				itr = m_sinks.erase(itr);
			}
			else
			{
				++itr;
			}
		}
	}

	void detach(event_type type, void* handler)
	{
		event_sink& sink = m_sinks.at(type.m_hash);
		sink.remove_pipe(handler);

		if (sink.m_pipes.size() == 0)
		{
			m_sinks.erase(type.m_hash);
		}
	}

	void send(event_type type, void* event)
	{
		auto itr = m_sinks.find(type.m_hash); // might cause a problem on large throughput of events
		if (itr != m_sinks.end())
		{
			itr->second.send(event);
		}

		for (event_manager* child : m_children)
		{
			child->send(type, event);
		}
	}

	// template headers

	template<typename _t>
	void detach(void* handler_ptr)
	{
		detach(make_event<_t>(), handler_ptr);
	}

	template<typename _t>
	void send(_t&& event)
	{
		send(make_event<_t>(), (void*)&event);
	}

	// adding / remove child event_managers

	void attach_child(event_manager* child)
	{
		for (event_manager* c : m_children)
		{
			if (child == c)
			{
				assert(false && "Child already added");
				return;
			}
		}

		m_children.push_back(child);
	}

	void detach_child(event_manager* child)
	{
		for (auto itr = m_children.begin(); itr != m_children.end(); ++itr)
		{
			if (*itr == child)
			{
				m_children.erase(itr);
				return;
			}
		}

		assert(false && "Child not in manager");
	}
};

struct event_queue
{
	struct queued_event_base
	{
		virtual ~queued_event_base() = default;
		virtual void send(event_manager* manager) = 0;
	};

	template<typename _e>
	struct queued_event : queued_event_base
	{
		const char* m_where;
		event_type m_type;
		_e m_event;

		queued_event(event_type type, const char* where, const _e& event)
			: m_where (where)
			, m_type  (type)
			, m_event (event)
		{}

		void send(event_manager* manager)
		{
			manager->send(m_type, &m_event);
		}
	};

	event_manager* m_manager;
	tsque<queued_event_base*> m_queue;

	const char* m_where_current;

	event_queue(
		event_manager* manager
	)
		: m_manager       (manager)
		, m_where_current (nullptr)
	{}

	template<typename _e>
	void send(const _e& event)
	{
		queued_event<_e>* qe = new queued_event<_e>(make_event<_e>(), m_where_current, event); // should use pool...
		m_queue.push_back(qe);
	}

	void execute()
	{
		while (m_queue.size() > 0)
		{
			queued_event_base* e = m_queue.pop_front();
			e->send(m_manager);
			delete e;
		}
	}
};