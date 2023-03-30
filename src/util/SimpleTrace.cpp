#include "util/SimpleTrace.h"

// should use this here, or make time a util
#include "app/Time.h"

#include "ext/serial/serial_json.h"

SimpleTraceScope::SimpleTraceScope(SimpleTrace* trace)
{
    m_event.ts = Time::TotalTimeNow();
    m_trace = trace;
}

SimpleTraceScope::~SimpleTraceScope()
{
    m_event.dur = (Time::TotalTimeNow() - m_event.ts) * 1000.f;
    m_trace->Report(m_event);
}

void SimpleTrace::Reset()
{
    report = {};
}

void SimpleTrace::Report(const SimpleTraceEvent& event)
{
    report.traceEvents.push_back(event);
}

void SimpleTrace::GenerateReport(std::ostream& stream) const
{
    json_writer(stream).write(report);
}

SimpleTrace* SimpleTrace::GetInstance()
{
    static SimpleTrace trace;
    return &trace;
}

SimpleTraceScope wTimeScope()
{
    return SimpleTraceScope(SimpleTrace::GetInstance());
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
