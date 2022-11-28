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

	void send(void* event);
};

template<typename _e>
struct event_pipe_wrapper
{
	event_pipe& m_pipe;

	event_pipe_wrapper(event_pipe& pipe) 
		: m_pipe(pipe)
	{}

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

	void remove_pipe(void* handler);

	void send(void* event);
};

struct EventBus
{
private:
	std::unordered_map<hash_t, event_sink> m_sinks;
	std::vector<EventBus*> m_children; // doesnt own
	EventBus* m_parent; // to be able to detach

public:
	EventBus();

	template<typename _e, typename _h>
	event_pipe_wrapper<_e> Attach(_h* handler_ptr)
	{
		event_sink& sink = m_sinks[make_event<_e>().m_hash];
		event_pipe& pipe = sink.add_pipe<_e, _h>(handler_ptr);
		return event_pipe_wrapper<_e>(pipe);
	}

	void Detach(void* handler);
	void Detach(event_type type, void* handler);

	void Send(event_type type, void* event);

	// adding / remove child event_managers

	void ChildAttach(EventBus* child);
	void ChildDetach(EventBus* child);

	void DetachFromParent();

	// template headers

	template<typename _e>
	void Detach(void* handler_ptr)
	{
		detach(make_event<_e>(), handler_ptr);
	}

	template<typename _e>
	void Send(_e&& event)
	{
		Send(make_event<_e>(), (void*)&event);
	}
};

struct EventQueue
{
	struct queued_event_base
	{
		virtual ~queued_event_base() = default;
		virtual void send(EventBus* manager) = 0;
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

		void send(EventBus* manager)
		{
			manager->Send(m_type, &m_event);
		}
	};

	EventBus* m_manager;
	tsque<queued_event_base*> m_queue;
	const char* m_where_current;

	EventQueue(
		EventBus* manager
	);

	void Execute();

	template<typename _e>
	void Send(const _e& event)
	{
		queued_event<_e>* qe = new queued_event<_e>(make_event<_e>(), m_where_current, event); // should use pool...
		m_queue.push_back(qe);
	}
};