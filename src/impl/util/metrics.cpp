#include "util/metrics.h"
#include "ext/Time.h" // this should be moved to util

met_series root = met_series("root", nullptr);
met_series* current = &root;

void met_report_begin(const char* name)
{
	met_series* s = nullptr;

	for (met_series* ser : current->children) // find series with name
	if (ser->name == name) {
		s = ser;
		break;
	}

	if (s == nullptr) // if none exist create a new one
	{
		s = new met_series(name, current);
		current->children.push_back(s);
	}

	current = s;
}

void met_report_end(float begin, float end)
{
	current->durations.push({begin, end - begin});
	current = current->parent;
		
	if (current->durations.size() > 1000)
	{
		current->durations.pop();
	}
}

const std::vector<met_series*>& met_get_series()
{
	return root.children;
}

met_scope_timer::met_scope_timer(const char* name)
	: begin (Time::TotalTimeNow())
	, end   (0.f)
	, name  (name)
{
	met_report_begin(name);
}

met_scope_timer::~met_scope_timer()
{
	end = Time::TotalTimeNow();
	met_report_end(begin, end);
}