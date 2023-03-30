#pragma once

#include <string>
#include <vector>

// store a list of trace events for google trace viewer

struct SimpleTraceEvent
{
    int pid;
    int tid;
    float ts;
    float dur;
    std::string ph = "X";
    std::string name;
    
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
    SimpleTraceScope(SimpleTrace* trace);
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
};

SimpleTraceScope wTimeScope();

void wInitSimpleTrace();
