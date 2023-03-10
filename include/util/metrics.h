#pragma once

#include <vector>
#include <queue>

struct met_duration
{
	float begin;
	float time;
};

struct met_series
{
	met_series* parent;
	const char* name;

	std::vector<met_series*> children;
	std::queue<met_duration> durations;

	met_series(const char* name, met_series* parent)
		: name   (name)
		, parent (parent)
	{}
};

struct met_scope_timer
{
private:
	float begin, end;
	const char* name;

public:
	met_scope_timer(const char* name);
	~met_scope_timer();
};

void met_report_begin(const char* name);
void met_report_end(float begin, float end);

const std::vector<met_series*>& met_get_series();