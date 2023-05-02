#include "util/SimpleTrace.h"

// should use this here, or make time a util
#include "Time.h"
#include "ext/serial/serial_json.h"

#include "SDL_thread.h"

#include <thread>

float millis()
{
    return Time::TotalTimeNow() * 1000.f;
}

SimpleTraceScope::SimpleTraceScope(const char* name, SimpleTrace* trace)
{
    unsigned long tid = SDL_ThreadID();
    unsigned long pid = 0;
    
    m_trace = trace;
    
    m_event.name = name;
    m_event.tid = tid;
    m_event.pid = pid;
    m_event.ts = millis();
}

SimpleTraceScope::~SimpleTraceScope()
{
    m_event.dur = millis() - m_event.ts;
    m_trace->Report(m_event);
}

void SimpleTrace::Reset()
{
    events.clear();
}

void SimpleTrace::Report(const SimpleTraceEvent& event)
{
    events.push_back(event);
}

void SimpleTrace::GenerateReport(std::ostream& stream) const
{
    SimpleTraceReport report;
    report.traceEvents = std::vector<SimpleTraceEvent>(events.begin(), events.end());

    json_writer(stream).write(report);
}

SimpleTrace* SimpleTrace::GetInstance()
{
    static SimpleTrace trace;
    return &trace;
}

SimpleTraceScope wTimeScope(const char* name)
{
    return SimpleTraceScope(name, SimpleTrace::GetInstance());
}

void wInitSimpleTrace()
{
    meta::describe<SimpleTraceEvent>()
        .member<&SimpleTraceEvent::pid>("pid")
        .member<&SimpleTraceEvent::tid>("tid")
        .member<&SimpleTraceEvent::ts>("ts")
        .member<&SimpleTraceEvent::dur>("dur")
        .member<&SimpleTraceEvent::ph>("ph")
        .member<&SimpleTraceEvent::name>("name");
    
    meta::describe<SimpleTraceReport>()
        .member<&SimpleTraceReport::traceEvents>("traceEvents");
}
