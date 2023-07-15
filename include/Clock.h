#pragma once

#include "util/context.h"
#include <stddef.h>
#include <chrono>

namespace Time
{
    using clock     = std::chrono::high_resolution_clock;
    using timepoint = std::chrono::time_point<clock>;
    using duration  = clock::duration;

    struct TimeContext : wContext
    {
        timepoint chrono_start = clock::now();
        timepoint chrono_now   = clock::now();
        duration  chrono_delta = clock::duration::zero();

        size_t ticks = 0;
        
        float time_scale = 1.0f;
        float time_fixed = 0.02f;

        float total_time        = 0.0f;
        float total_time_scaled = 0.0f;

        // for optimizations, don't do math in function calls

        float current_delta = 0.f;
        float current_delta_scaled = 0.f;
        float current_fixed_scaled = 0.f;
    };

    wContextDecl(TimeContext);

	void UpdateTime();

	void SetFixedTime(float duration);
	void SetTimeScale(float duration);

	size_t Ticks();

	float TimeScale();
	float FixedTime();

	float TotalTime();
	float DeltaTime();
	
	float RawTotalTime();
	float RawDeltaTime();
	float RawFixedTime();

	float DeltaTimeNow();
	float TotalTimeNow();
}

struct wTimer {
    float begin;

    wTimer() { begin = Time::TotalTimeNow(); }

    float Seconds() const {
        return Time::TotalTimeNow() - begin;
    }
};