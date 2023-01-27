#pragma once

// new event system
// should focus on the order of function execution

// main goal:
// should allow for handlers to classes without an interface for every type of function
// and events arent an interface

#include <vector>
#include <unordered_map>
#include <memory>

template<typename _event>
using _event_ff = void (*)(_event&);

template<typename _event, typename _instance>
using _event_mf = void (_instance::*)(_event&);

class EventPipeBase
{
public:
    virtual ~EventPipeBase() = default;
    
    virtual void Send(void* event) const = 0;
    virtual const void* GetInstance() const = 0;
};

template<typename _event>
class EventPipe : public EventPipeBase
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

class EventSink
{
public:
    void AttachPipe(std::shared_ptr<EventPipeBase> pipe);
    
    // Remove a pipe by refernce, does not delete
    void DetachPipe(std::shared_ptr<EventPipeBase> pipe);
    
    // Remove a pipe by searching for a bound instance or free function pointer
    void DetachPipe(const void* instanceOrFunctionPointer);
    
    void Send(void* event) const;
    
    int GetNumberOfPipes() const;
    
private:
    std::vector<std::shared_ptr<EventPipeBase>> m_pipes;
};

using EventType = size_t;

template<typename _event>
EventType GetEventType()
{
    return typeid(_event).hash_code();
}

// Hold a reference to a bound event. Used for detaching events
class Event final
{
public:
    Event();
    Event(std::shared_ptr<EventPipeBase> pipe, const EventType& type);
    
private:
    std::shared_ptr<EventPipeBase> pipe;
    EventType type;
    
    friend class EventBus;
};

class EventBus
{
public:
	EventBus();

	template<typename _event, typename _instance>
    Event Attach(_instance* instance)
	{
        std::shared_ptr<EventPipeBase> pipe = std::make_shared<EventPipeMemberFunction<_event, _instance>>(instance);
        EventType type = GetEventType<_event>();
        
        m_sinks[type].AttachPipe(pipe);
        
        return Event(pipe, type);
	}
    
    template<typename _event>
    Event Attach(_event_ff<_event> function)
    {
        std::shared_ptr<EventPipeBase> pipe = std::make_shared<EventPipeFreeFunction<_event>>(function);
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
    void Send(_event& event)
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

class EventQueue
{
public:
    EventQueue();
    EventQueue(EventBus* bus);
    
    EventBus* GetBus() const;
    
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
        
        void Send(EventBus* bus) override
        {   
            bus->Send(event);
        }
    };
    
    EventBus* m_bus;
    std::vector<QueuedEvent*> m_queue;
};
