#pragma once

#include <string>
#include <vector>
#include <mutex>

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
    // should use concurrent vec
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
    SimpleTraceReport report;
    std::mutex mut;
};

SimpleTraceScope wTimeScope(const char* name);
void wInitSimpleTrace();
