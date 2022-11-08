#include "app/Time.h"
#include <chrono>
#include <vector>

namespace Time
{
    TimeContext* ctx;

    void CreateContext()
    {
        DestroyContext();
        ctx = new TimeContext();
    }

    void DestroyContext()
    {
        delete ctx;
    }

    TimeContext* GetContext()
    {
        return ctx;
    }

    void SetCurrentContext(TimeContext* context)
    {
        ctx = context;
    }

	void UpdateTime()
	{
        // could put this in the context to not deref so much...
        
		ctx->ticks++;

        ctx->current_delta        = ctx->chrono_delta.count() / 1000000000.0f;
        ctx->current_delta_scaled = ctx->time_scale * ctx->current_delta;
        ctx->current_fixed_scaled = ctx->time_scale * ctx->time_fixed;

        ctx->total_time        += ctx->current_delta;
        ctx->total_time_scaled += ctx->current_delta_scaled;

        ctx->chrono_delta = clock::now() - ctx->chrono_now;
        ctx->chrono_now   = clock::now();
	}

	void SetTimeScale(float duration) { ctx->time_scale = duration; log_app("i~Set time scale to %f", duration); }
	void SetFixedTime(float duration) { ctx->time_fixed = duration; log_app("i~Set fixed time step to %f", duration); }

    size_t Ticks() { return ctx->ticks; }
	
	float TimeScale() { return ctx->time_scale; }
	float FixedTime() { return ctx->current_fixed_scaled; }

	float TotalTime() { return ctx->total_time_scaled; }
	float DeltaTime() { return ctx->current_delta_scaled; }
	
	float RawTotalTime() { return ctx->total_time; }
	float RawFixedTime() { return ctx->time_fixed; }
	float RawDeltaTime() { return ctx->current_delta; }
	
	float DeltaTimeNow() { return (clock::now() - ctx->chrono_now)  .count() / 1000000000.0f; }
	float TotalTimeNow() { return (clock::now() - ctx->chrono_start).count() / 1000000000.0f; } // would be good to remove these divisions and use the std::ratio construct
}
