#pragma once

#include "ext/Time.h"
#include <unordered_map>
#include <queue>

struct metrics_log // not thread safe, but could be
{
public:
	struct duration
	{
		float begin;
		float time;
	};

	struct series 
	{
		series* parent;
		std::vector<series*> children;
		std::queue<duration> durations;
		const char* name;

		series(const char* name, series* parent)
			: name   (name)
			, parent (parent)
		{}
	};

private:
	series root = series("root", nullptr);
	series* current = &root;

public:
	void report_begin(const char* name)
	{
		series* s = nullptr;

		for (series* ser : current->children) // find series with name
		if (ser->name == name) {
			s = ser;
			break;
		}

		if (s == nullptr) // if none exist create a new one
		{
			s = new series(name, current);
			current->children.push_back(s);
		}

		current = s;
	}

	void report_metric(float begin, float end)
	{
		current->durations.push({begin, end - begin});
		current = current->parent;
		
		if (current->durations.size() > 1000)
		{
			current->durations.pop();
		}
	}

	const std::vector<series*>& get_series()
	{
		return root.children;
	}
};

struct scope_timer
{
	float begin, end;
	const char* name;

private:
	metrics_log* owner;

public:
	scope_timer(const char* name, metrics_log* owner)
		: begin (Time::TotalTimeNow())
		, end   (0.f)
		, name  (name)
		, owner (owner)
	{
		owner->report_begin(name);
	}

	~scope_timer()
	{
		end = Time::TotalTimeNow();
		owner->report_metric(begin, end);
	}
};