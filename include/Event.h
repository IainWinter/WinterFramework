#pragma once

// An event bus & queue that allows binding to free functions and member functions
// to specific instances

#include <vector>
#include <unordered_map>
#include <memory>

template<typename _event>
using _event_ff = void (*)(_event&);

template<typename _event, typename _instance>
using _event_mf = void (_instance::*)(_event&);

//
//  Top interface representing an event pipe. Event pipes hold the function pointer
//      to the handler
//
class IEventPipe
{
public:
    virtual ~IEventPipe() = default;
    
    virtual void Send(void* event) const = 0;
    virtual const void* GetInstance() const = 0;
};

//
//  Casts generic event data to the concrete event type of this pipe
//
template<typename _event>
class EventPipe : public IEventPipe
{
public:
    virtual ~EventPipe() = default;
    
    void Send(void* event) const override
    {
        _Send(*static_cast<_event*>(event));
    }
    
protected:
    virtual void _Send(_event& event) const = 0;
};

//
//  Event pipe for free functions.
//      Can bind to functions with the signature: void <name>(_event&);
//
template<typename _event>
class EventPipeFreeFunction final : public EventPipe<_event>
{
public:
    EventPipeFreeFunction(_event_ff<_event> func)
        : m_func (func)
    {}
    
    const void* GetInstance() const override
    {
        return (void*)m_func;
    }
    
protected:
    void _Send(_event& event) const override
    {
        m_func(event);
    }
    
private:
    _event_ff<_event> m_func;
};

//
//  Event pipe for member functions.
//      Can bind to functions with the signature: void _instance::on(_event&);
//
template<typename _event, typename _instance>
class EventPipeMemberFunction final : public EventPipe<_event>
{
public:
    EventPipeMemberFunction(_instance* instance)
        : m_instance (instance)
        , m_func     (&_instance::on)
    {}
    
    const void* GetInstance() const override
    {
        return (void*)m_instance;
    }
    
protected:
    void _Send(_event& event) const override
    {
        (m_instance ->* m_func) (event);
    }
    
private:
    _event_mf<_event, _instance> m_func;
    _instance* m_instance;
};

//
//  Groups event pipes by event type
//
class EventSink
{
public:
    void AttachPipe(std::shared_ptr<IEventPipe> pipe);
    void DetachPipe(std::shared_ptr<IEventPipe> pipe);
    
    // Remove a pipe by searching for a bound instance or free function pointer
    void DetachPipe(const void* instanceOrFunctionPointer);
    
    void Send(void* event) const;
    
    int GetNumberOfPipes() const;
    
private:
    std::vector<std::shared_ptr<IEventPipe>> m_pipes;
};

//
//  Runtime identifier for event types
//
using EventType = size_t;

template<typename _event>
EventType GetEventType()
{
    return typeid(_event).hash_code();
}

//
//  Hold a reference to a bound event. Used for detaching events
//
class Event final
{
public:
    Event();
    Event(std::shared_ptr<IEventPipe> pipe, const EventType& type);
    
private:
    std::shared_ptr<IEventPipe> pipe;
    EventType type;
    
    friend class EventBus;
};

//
//  A group of event sinks and links to child buses
//      - Events are sent to all children, but not parent
//      - To bubble up event, user needs a link to the parent bus
//
class EventBus
{
public:
	EventBus();

	template<typename _event, typename _instance>
    Event Attach(_instance* instance)
	{
        std::shared_ptr<IEventPipe> pipe = std::make_shared<EventPipeMemberFunction<_event, _instance>>(instance);
        EventType type = GetEventType<_event>();
        
        m_sinks[type].AttachPipe(pipe);
        
        return Event(pipe, type);
	}
    
    template<typename _event>
    Event Attach(_event_ff<_event> function)
    {
        std::shared_ptr<IEventPipe> pipe = std::make_shared<EventPipeFreeFunction<_event>>(function);
        EventType type = GetEventType<_event>();
        
        m_sinks[type].AttachPipe(pipe);
        
        return Event(pipe, type);
    }
    
    void Detach(const Event& event);
    void Detach(const void* instanceOrFunctionPointer);
    void Detach(EventType type, const void* instanceOrFunctionPointer);
    
    template<typename _event>
    void Detach(const void* instanceOrFunctionPointer)
    {
        Detach(GetEventType<_event>(), instanceOrFunctionPointer);
    }
    
    void Send(EventType type, void* event);
    
    template<typename _event>
    void Send(_event&& event)
    {
        Send(GetEventType<_event>(), &event);
    }
    
	// adding / remove child event_managers

	void ChildAttach(EventBus* child);
	void ChildDetach(EventBus* child);
    
	void DetachFromParent();
    
private:
    std::unordered_map<EventType, EventSink> m_sinks;

    // Parenting
    
    std::vector<EventBus*> m_children;
    EventBus* m_parent;
};

//
//  A queue of events that are executed in a batch. todo: Add make thread safe
//
class EventQueue
{
public:
    EventQueue();
    EventQueue(EventBus* bus);
    
    void Execute();
    
    template<typename _event>
    void Send(const _event& event, const char* where = "")
    {
        QueuedEventWrapped<_event>* queued = new QueuedEventWrapped<_event>(event, where);
        m_queue.push_back(queued);
    }
    
private:
    struct QueuedEvent
    {
        virtual ~QueuedEvent() = default;
        virtual void Send(EventBus* bus) = 0;
    };
    
    template<typename _event>
    struct QueuedEventWrapped : QueuedEvent
    {
        _event event;
        const char* where;
        
        QueuedEventWrapped(const _event& event, const char* where)
            : event (event)
            , where (where)
        {}
        
        void Send(EventBus* _bus) override
        {   
            _bus->Send(event);
        }
    };
    
public:
    EventBus* bus;

private:
    std::vector<QueuedEvent*> m_queue;
};
