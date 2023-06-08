#include "Event.h"
#include <assert.h>

void EventSink::AttachPipe(std::shared_ptr<IEventPipe> pipe)
{
    DetachPipe(pipe);
    m_pipes.push_back(pipe);
}

void EventSink::DetachPipe(std::shared_ptr<IEventPipe> pipe)
{
    auto itr = std::find(m_pipes.begin(), m_pipes.end(), pipe);
    if (itr != m_pipes.end())
        m_pipes.erase(itr);
}

void EventSink::DetachPipe(const void* instanceOrFunctionPointer)
{
    for (const std::shared_ptr<IEventPipe>& pipe : m_pipes)
    {
        if (pipe->GetInstance() == instanceOrFunctionPointer)
        {
            DetachPipe(pipe);
            break;
        }
    }
}

void EventSink::Send(void* event) const
{
    for (const std::shared_ptr<IEventPipe>& pipe : m_pipes)
    {
        pipe->Send(event);
    }
}

int EventSink::GetNumberOfPipes() const
{
    return m_pipes.size();
}

Event::Event()
    : pipe (nullptr)
    , type ()
{}

Event::Event(std::shared_ptr<IEventPipe> pipe, const EventType& type)
    : pipe (pipe)
    , type (type)
{}

EventBus::EventBus()
	: m_parent (nullptr)
{}

void EventBus::Detach(const Event& event)
{
    auto sink = m_sinks.find(event.type);

    sink->second.DetachPipe(event.pipe);
        
    if (sink->second.GetNumberOfPipes() == 0)
        m_sinks.erase(sink);
}

void EventBus::Detach(const void* instanceOrFunctionPointer)
{
    for (auto itr = m_sinks.begin(); itr != m_sinks.end();)
    {
        itr->second.DetachPipe(instanceOrFunctionPointer);

        if (itr->second.GetNumberOfPipes() == 0) // remove empty sinks
            itr = m_sinks.erase(itr);
        else
            ++itr;
    }
}

void EventBus::Detach(EventType type, const void* instanceOrFunctionPointer)
{
    auto itr = m_sinks.find(type);

    if (itr != m_sinks.end())
        itr->second.DetachPipe(instanceOrFunctionPointer);

    if (itr->second.GetNumberOfPipes() == 0)
        m_sinks.erase(itr);
}

void EventBus::Send(EventType type, void* event)
{
	auto itr = m_sinks.find(type); // might cause a problem on large throughput of events

	if (itr != m_sinks.end())
		itr->second.Send(event);

	// Loop on an index because a child could be removed from the bus in
    // the event handler
    
	for (int i = 0; i < m_children.size(); i++)
	{
        m_children.at(i)->Send(type, event);
	}
}

void EventBus::ChildAttach(EventBus* child)
{
    // todo: check for loops

	for (EventBus* c : m_children)
		if (child == c)
			return;

	m_children.push_back(child);
	child->m_parent = this;
}

void EventBus::ChildDetach(EventBus* child)
{
	for (auto itr = m_children.begin(); itr != m_children.end(); ++itr)
	{
		if (*itr == child)
		{
			(*itr)->m_parent = nullptr;
			m_children.erase(itr);

			break;
		}
	}
}

void EventBus::DetachFromParent()
{
	if (m_parent)
		m_parent->ChildDetach(this);
}

EventQueue::EventQueue()
    : bus (nullptr)
{}

EventQueue::EventQueue(EventBus* bus)
    : bus (bus)
{}

bool EventQueue::Execute()
{
    bool hasEvents = m_queue.size() > 0;

    for (int i = 0; i < m_queue.size(); i++)
    {
        QueuedEvent* event = m_queue.at(i);
        event->Send(bus);
        delete event;
    }

    m_queue.clear();

    return hasEvents;
}
