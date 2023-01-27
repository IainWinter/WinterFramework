#pragma once

#include <vector>
#include <unordered_map>

// Event function types

// Free functions need to be of the signature void <name>(<event_type>&)
// Member functions need to be of the signature void on(<event_type>&)

template<typename _event>
using _event_func_f = void (*)(_t&);

template<typename _event, typename _instance>
using _event_func_i = void (_instance::*on)(_event&);

class EventPipe
{
public:
    virtual void Send(void* event) const = 0;
};

template<typename _event>
class EventPipeFreeFunction : public EventPipe
{
public:
    using _func_t = _event_func_f<_event>;
    
public:
    EventPipeFreeFunction(_func_t func)
        : m_func(func)
    {}
    
    void Send(void* event) const override
    {
        m_func (*static_cast<_event*>(event));
    }
    
private:
    _func_t m_func;
};

template<typename _event, typename _instance>
class EventPipeMemberFunction : public EventPipe
{
public:
    using _func_t = _event_func_i<_event, _instance>;
    
public:
    EventPipeMemberFunction(_instance instance)
        : m_instance (instance)
        , m_func     (&_instance::on(_event&))
    {}
    
    void Send(void* event) const override
    {
        m_instance ->* m_func (*static_cast<_event*>(event));
    }
    
private:
    _func_t m_func;
    _instance m_instance;
}

class EventSink
{
private:
    size_t m_type;
    std::vector<EventPipe*> m_pipes;
    
public:
    EventSink(size_t type)
        : m_type (type)
    {}
    
    void Attach(EventPipe* pipe)
    {
        m_pipes.push_back(pipe);
    }
    
    void Detach(EventPipe* pipe)
    {
        m_pipes.erase(std::find(m_pipes.begin(), m_pipes.end(), pipe));
    }
    
    void Send(void* event)
    {
        for (const EventPipe* pipe : m_pipes)
        {
            pipe->Send(event);
        }
    }
}

class EventBus
{
public:
    
    template<typename _event>
    void Attach(_event_func_f<_event> function)
    {
        Assure<_event>().Attach(new EventPipeFreeFunction<_event>(function));
    }
    
    template<typename _event, typename _instance>
    void Attach(_instance* instance)
    {
        Assure<_event>().Attach(new EventPipeFreeFunction<_event>(function));
    }
    
private:
    template<typename _event>
    EventSink* Assure()
    {
        size_t type_id = typeid(_event).hash_code();
        
        if (m_sinks.count(type_id) == 0)
            m_sinks.emplace(new EventSink(type_id));
        
        return m_sinks.at(type_id);
    }
    
private:
    std::unordered_map<size_t, EventSink> m_sinks;
}
