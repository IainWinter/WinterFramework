#pragma once

#include <string>
#include <vector>
#include <mutex>

#include <concurrent_vector.h>

// store a list of trace events for google trace viewer

struct SimpleTraceEvent
{
    int pid = 0;
    int tid = 0;
    float ts = 0.0;
    float dur = 0.0;
    std::string ph = "X";
    std::string name = "Unset Name";
        
    // could have custom object of args
};

struct SimpleTraceReport
{
    std::vector<SimpleTraceEvent> traceEvents;
};

class SimpleTrace;

class SimpleTraceScope
{
public:
    SimpleTraceScope(const char* name, SimpleTrace* trace);
    ~SimpleTraceScope();
    
private:
    SimpleTrace* m_trace;
    SimpleTraceEvent m_event;
};

class SimpleTrace
{
public:
    void Reset();
    
    void Report(const SimpleTraceEvent& event);
    void GenerateReport(std::ostream& stream) const;
    
    static SimpleTrace* GetInstance();
    
private:
    SimpleTrace() = default;
    
private:
    concurrency::concurrent_vector<SimpleTraceEvent> events;
};

#define wTIME_SCOPE(name) auto timer = wTimeScope(name)

SimpleTraceScope wTimeScope(const char* name);
void wInitSimpleTrace();
