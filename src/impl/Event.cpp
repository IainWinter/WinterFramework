#pragma once

#include "Event.h"

void event_pipe::send(void* event)
{
	if (!m_only_if || m_only_if(event))
	{
		m_send(event);
	}
}

void event_sink::remove_pipe(void* handler)
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

void event_sink::send(void* event)
{
	for (event_pipe& pipe : m_pipes)
	{
		pipe.send(event);
	}
}

void event_manager::detach(void* handler)
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

void event_manager::detach(event_type type, void* handler)
{
	event_sink& sink = m_sinks.at(type.m_hash);
	sink.remove_pipe(handler);

	if (sink.m_pipes.size() == 0)
	{
		m_sinks.erase(type.m_hash);
	}
}

void event_manager::send(event_type type, void* event)
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

void event_manager::attach_child(event_manager* child)
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

void event_manager::detach_child(event_manager* child)
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

event_queue::event_queue(
	event_manager* manager
)
	: m_manager       (manager)
	, m_where_current (nullptr)
{}

void event_queue::execute()
{
	while (m_queue.size() > 0)
	{
		queued_event_base* e = m_queue.pop_front();
		e->send(m_manager);
		delete e;
	}
}