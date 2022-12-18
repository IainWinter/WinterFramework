#include "app/InputMap.h"

namespace Input
{
	InputContext* ctx;

	void CreateContext()
	{
		DestroyContext();
		ctx = new InputContext();
	}

	void DestroyContext()
	{
		delete ctx;
	}

	void SetCurrentContext(InputContext* context)
	{
		ctx = context;
	}

	InputContext* GetContext()
	{
		return ctx;
	}

	float GetButton(InputName button)
	{
		return GetAxis(button).x;
	}

	vec2 GetAxisNoRecurse(InputName axis)
	{
		vec2 out = vec2(0.f);

		auto itr = ctx->Axes.find(axis);
		if (itr != ctx->Axes.end())
		{
			for (const auto& component : itr->second.components)
			{
				out += ctx->State[component.first] * component.second;
			}

			if (length(out) < itr->second.deadzone)
			{
				out = vec2(0.f);
			}
		}

		return limit(out, 1.f);
	}

	vec2 GetAxis(InputName axis)
	{
		vec2 out = GetAxisNoRecurse(axis);

		auto vitr = ctx->VirtualAxes.find(axis);
		if (vitr != ctx->VirtualAxes.end())
		{
			for (const auto& child : vitr->second.children)
			{
				out += GetAxisNoRecurse(child);
			}
		}

		return limit(out, 1.f);
	}

	void CreateAxis(InputName name)
	{
		if (AxisExists(name))
		{
			log_app("w~Axis already exists. %s", name);
			return;
		}

		ctx->Axes.emplace(name, InputAxis{});
	}

	void CreateVirtualAxis(InputName name)
	{
		if (VirtualAxisExists(name))
		{
			log_app("w~Virtual axis already exists. %s", name);
			return;
		}

		ctx->VirtualAxes.emplace(name, VirtualAxis{});
	}

	bool AxisExists(InputName axis)
	{
		return ctx->Axes.find(axis) != ctx->Axes.end();
	}

	bool VirtualAxisExists(InputName axis)
	{
		return ctx->VirtualAxes.find(axis) != ctx->VirtualAxes.end();
	}

	void SetDeadzone(InputName axis, float deadzone)
	{
		if (!AxisExists(axis))
		{
			log_app("w~Axis doesn't exist. %s", axis);
			return;
		}

		ctx->Axes[axis].deadzone = deadzone;
	}

	void SetVirtualAxisComponent(InputName axis, InputName component)
	{
		if (!VirtualAxisExists(axis))
		{
			log_app("w~Virtual axis doesn't exist. %s", axis);
			return;
		}

		if (!AxisExists(component))
		{
			log_app("w~Axis doesn't exist. %s", axis);
			return;
		}

		ctx->VirtualAxes[axis].children.push_back(component);

		for (const auto& c : ctx->Axes[component].components) // we need to update mapping 
		{
			ctx->Mapping[c.first] = axis;
		}
	}

	// translation helpers

	int GetCode(KeyboardInput scancode)
	{
		return (int)scancode;
	}

	int GetCode(ControllerInput input)
	{
		return 1 + (int)input + SDL_NUM_SCANCODES;
	}

	InputName GetMapping(int code)
	{
		auto itr = ctx->Mapping.find(code);
		return itr != ctx->Mapping.end() ? itr->second : nullptr;
	}

	InputName GetMapping(KeyboardInput scancode)
	{
		return GetMapping(GetCode(scancode));
	}

	InputName GetMapping(ControllerInput scancode)
	{
		return GetMapping(GetCode(scancode));
	}

	void SetAxisComponent(InputName axis, int code, vec2 weight)
	{
		if (!AxisExists(axis))
		{
			log_app("w~Axis doesn't exist. %s", axis);
			return;
		}

		ctx->Axes[axis].components.emplace(code, weight);
		ctx->Mapping.emplace(code, axis);
	}

	void SetAxisComponent(InputName axis, KeyboardInput scancode, vec2 weight)
	{
		SetAxisComponent(axis, GetCode(scancode), weight);
	}

	void SetAxisComponent(InputName axis, ControllerInput input, vec2 weight)
	{
		SetAxisComponent(axis, GetCode(input), weight);
	}

	// internal

	void SetState(int code, float state)
	{
		if (code < 0 || code >= NUMBER_OF_STATES)
		{
			log_app("e~Tried to set state of code that is invalid. %d -> %f", code, state);
			return;
		}

		ctx->State[code] = state;
	}
}