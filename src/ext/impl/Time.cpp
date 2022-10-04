#include "ext/Time.h"
#include <chrono>
#include <vector>

namespace Time
{
	using namespace std;
	using namespace chrono;

	auto chrono_start = high_resolution_clock::now();
	auto chrono_now   = high_resolution_clock::now();
	auto chrono_delta = high_resolution_clock::duration::zero();

	size_t ticks    = 0;
	
	float time_scale = 1.0f;
	float time_fixed = 0.02f;

	float total_time        = 0.0f;
	float total_time_scaled = 0.0f;

	// for optimizations, dont do math in function calls

	float current_delta = 0.f; 
	float current_delta_scaled = 0.f;
	float current_fixed_scaled = 0.f;

	void UpdateTime()
	{
		ticks++;

		current_delta        = chrono_delta.count() / 1000000000.0f;
		current_delta_scaled = time_scale * current_delta;
		current_fixed_scaled = time_scale * time_fixed;

		total_time        += current_delta;
		total_time_scaled += current_delta_scaled;

		chrono_delta = high_resolution_clock::now() - chrono_now;
		chrono_now   = high_resolution_clock::now();
	}

	void SetTimeScale(float duration) { time_scale = duration; log_app("i~Set time scale to %f", duration); }
	void SetFixedTime(float duration) { time_fixed = duration; log_app("i~Set fixed time step to %f", duration); }

    size_t Ticks() { return ticks; }
	
	float TimeScale() { return time_scale; }
	float FixedTime() { return current_fixed_scaled; }

	float TotalTime() { return total_time_scaled; }
	float DeltaTime() { return current_delta_scaled; }
	
	float RawTotalTime() { return total_time; }
	float RawFixedTime() { return time_fixed; }
	float RawDeltaTime() { return current_delta; }
	
	float DeltaTimeNow() { return (high_resolution_clock::now() - chrono_now)  .count() / 1000000000.0f; }
	float TotalTimeNow() { return (high_resolution_clock::now() - chrono_start).count() / 1000000000.0f; } // would be good to remove these divisions and use the std::ratio construct
}
