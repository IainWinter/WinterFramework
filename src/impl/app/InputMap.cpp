#include "app/InputMap.h"

#include "Log.h"
#include "util/context.h"
#include "ext/serial/serial_json.h"

#include <fstream>

namespace Input
{
	InputContext::InputContext()
	{
		ViewportMin = vec2(0, 0);
		ViewportSize = vec2(1, 1);

		// Describe types

		meta::describe<VirtualAxis>()
			.name("VirtualAxis")
			.member<&VirtualAxis::limitToUnit>("limitToUnit")
			.member<&VirtualAxis::axes>("axes");

		meta::describe<InputAxis>()
			.name("InputAxis")
			.member<&InputAxis::deadzone>("deadzone")
			.member<&InputAxis::limitToUnit>("limitToUnit")
			.member<&InputAxis::components>("components");

		meta::describe<InputContext>()
			.name("InputContext")
			.member<&InputContext::VirtualAxes>("VirtualAxes")
			.member<&InputContext::Axes>("Axes")
			.member<&InputContext::Mapping>("Mapping");

		// populate inital states, kinda sucks to have to do this
		// but needed for iterating avalible states by using the keys of ctx->State

		State[GetInputCode(KEY_A)] = 0.f;
		State[GetInputCode(KEY_B)] = 0.f;
		State[GetInputCode(KEY_C)] = 0.f;
		State[GetInputCode(KEY_D)] = 0.f;
		State[GetInputCode(KEY_E)] = 0.f;
		State[GetInputCode(KEY_F)] = 0.f;
		State[GetInputCode(KEY_G)] = 0.f;
		State[GetInputCode(KEY_H)] = 0.f;
		State[GetInputCode(KEY_I)] = 0.f;
		State[GetInputCode(KEY_J)] = 0.f;
		State[GetInputCode(KEY_K)] = 0.f;
		State[GetInputCode(KEY_L)] = 0.f;
		State[GetInputCode(KEY_M)] = 0.f;
		State[GetInputCode(KEY_N)] = 0.f;
		State[GetInputCode(KEY_O)] = 0.f;
		State[GetInputCode(KEY_P)] = 0.f;
		State[GetInputCode(KEY_Q)] = 0.f;
		State[GetInputCode(KEY_R)] = 0.f;
		State[GetInputCode(KEY_S)] = 0.f;
		State[GetInputCode(KEY_T)] = 0.f;
		State[GetInputCode(KEY_U)] = 0.f;
		State[GetInputCode(KEY_V)] = 0.f;
		State[GetInputCode(KEY_W)] = 0.f;
		State[GetInputCode(KEY_X)] = 0.f;
		State[GetInputCode(KEY_Y)] = 0.f;
		State[GetInputCode(KEY_Z)] = 0.f;
		State[GetInputCode(KEY_1)] = 0.f;
		State[GetInputCode(KEY_2)] = 0.f;
		State[GetInputCode(KEY_3)] = 0.f;
		State[GetInputCode(KEY_4)] = 0.f;
		State[GetInputCode(KEY_5)] = 0.f;
		State[GetInputCode(KEY_6)] = 0.f;
		State[GetInputCode(KEY_7)] = 0.f;
		State[GetInputCode(KEY_8)] = 0.f;
		State[GetInputCode(KEY_9)] = 0.f;
		State[GetInputCode(KEY_0)] = 0.f;
		State[GetInputCode(KEY_Return)] = 0.f;
		State[GetInputCode(KEY_Escape)] = 0.f;
		State[GetInputCode(KEY_Backspace)] = 0.f;
		State[GetInputCode(KEY_Tab)] = 0.f;
		State[GetInputCode(KEY_Space)] = 0.f;
		State[GetInputCode(KEY_Minus)] = 0.f;
		State[GetInputCode(KEY_Equals)] = 0.f;
		State[GetInputCode(KEY_Bracket_Left)] = 0.f;
		State[GetInputCode(KEY_Bracket_Right)] = 0.f;
		State[GetInputCode(KEY_Backslash)] = 0.f;
		State[GetInputCode(KEY_SemiColon)] = 0.f;
		State[GetInputCode(KEY_Apostrophe)] = 0.f;
		State[GetInputCode(KEY_Grave)] = 0.f;
		State[GetInputCode(KEY_Comma)] = 0.f;
		State[GetInputCode(KEY_Period)] = 0.f;
		State[GetInputCode(KEY_Slash)] = 0.f;
		State[GetInputCode(KEY_Control_Left)] = 0.f;
		State[GetInputCode(KEY_Shift_Left)] = 0.f;
		State[GetInputCode(KEY_Alt_Left)] = 0.f;
		State[GetInputCode(KEY_GUI_Left)] = 0.f;
		State[GetInputCode(KEY_Control_Right)] = 0.f;
		State[GetInputCode(KEY_Shift_Right)] = 0.f;
		State[GetInputCode(KEY_Alt_Right)] = 0.f;
		State[GetInputCode(KEY_GUI_Right)] = 0.f;
		State[GetInputCode(KEY_Right)] = 0.f;
		State[GetInputCode(KEY_Left)] = 0.f;
		State[GetInputCode(KEY_Down)] = 0.f;
		State[GetInputCode(KEY_Up)] = 0.f;
		State[GetInputCode(KEY_Lock_Caps)] = 0.f;
		State[GetInputCode(KEY_Lock_Scroll)] = 0.f;
		State[GetInputCode(KEY_Lock_Number)] = 0.f;
		State[GetInputCode(KEY_PrintScreen)] = 0.f;
		State[GetInputCode(KEY_Pause)] = 0.f;
		State[GetInputCode(KEY_Insert)] = 0.f;
		State[GetInputCode(KEY_Home)] = 0.f;
		State[GetInputCode(KEY_Page_Up)] = 0.f;
		State[GetInputCode(KEY_Page_Down)] = 0.f;
		State[GetInputCode(KEY_Delete)] = 0.f;
		State[GetInputCode(KEY_End)] = 0.f;
		State[GetInputCode(KEY_Keypad_Divide)] = 0.f;
		State[GetInputCode(KEY_Keypad_Multiply)] = 0.f;
		State[GetInputCode(KEY_Keypad_Minus)] = 0.f;
		State[GetInputCode(KEY_Keypad_Plus)] = 0.f;
		State[GetInputCode(KEY_Keypad_Enter)] = 0.f;
		State[GetInputCode(KEY_Keypad_1)] = 0.f;
		State[GetInputCode(KEY_Keypad_2)] = 0.f;
		State[GetInputCode(KEY_Keypad_3)] = 0.f;
		State[GetInputCode(KEY_Keypad_4)] = 0.f;
		State[GetInputCode(KEY_Keypad_5)] = 0.f;
		State[GetInputCode(KEY_Keypad_6)] = 0.f;
		State[GetInputCode(KEY_Keypad_7)] = 0.f;
		State[GetInputCode(KEY_Keypad_8)] = 0.f;
		State[GetInputCode(KEY_Keypad_9)] = 0.f;
		State[GetInputCode(KEY_Keypad_0)] = 0.f;
		State[GetInputCode(KEY_Keypad_Period)] = 0.f;
		State[GetInputCode(KEY_Keypad_Equals)] = 0.f;
		State[GetInputCode(KEY_Function_1)] = 0.f;
		State[GetInputCode(KEY_Function_2)] = 0.f;
		State[GetInputCode(KEY_Function_3)] = 0.f;
		State[GetInputCode(KEY_Function_4)] = 0.f;
		State[GetInputCode(KEY_Function_5)] = 0.f;
		State[GetInputCode(KEY_Function_6)] = 0.f;
		State[GetInputCode(KEY_Function_7)] = 0.f;
		State[GetInputCode(KEY_Function_8)] = 0.f;
		State[GetInputCode(KEY_Function_9)] = 0.f;
		State[GetInputCode(KEY_Function_10)] = 0.f;
		State[GetInputCode(KEY_Function_11)] = 0.f;
		State[GetInputCode(KEY_Function_12)] = 0.f;
		State[GetInputCode(KEY_Function_13)] = 0.f;
		State[GetInputCode(KEY_Function_14)] = 0.f;
		State[GetInputCode(KEY_Function_15)] = 0.f;
		State[GetInputCode(KEY_Function_16)] = 0.f;
		State[GetInputCode(KEY_Function_17)] = 0.f;
		State[GetInputCode(KEY_Function_18)] = 0.f;
		State[GetInputCode(KEY_Function_19)] = 0.f;
		State[GetInputCode(KEY_Function_20)] = 0.f;
		State[GetInputCode(KEY_Function_21)] = 0.f;
		State[GetInputCode(KEY_Function_22)] = 0.f;
		State[GetInputCode(KEY_Function_23)] = 0.f;
		State[GetInputCode(KEY_Function_24)] = 0.f;

		State[GetInputCode(MOUSE_LEFT)] = 0.f;
		State[GetInputCode(MOUSE_MIDDLE)] = 0.f;
		State[GetInputCode(MOUSE_RIGHT)] = 0.f;
		State[GetInputCode(MOUSE_X1)] = 0.f;
		State[GetInputCode(MOUSE_X2)] = 0.f;
		State[GetInputCode(MOUSE_POS_X)] = 0.f;
		State[GetInputCode(MOUSE_POS_Y)] = 0.f;
		State[GetInputCode(MOUSE_VEL_X)] = 0.f;
		State[GetInputCode(MOUSE_VEL_Y)] = 0.f;
		State[GetInputCode(MOUSE_VEL_WHEEL_X)] = 0.f;
		State[GetInputCode(MOUSE_VEL_WHEEL_Y)] = 0.f;

		State[GetInputCode(cBUTTON_A)] = 0.f;
		State[GetInputCode(cBUTTON_B)] = 0.f;
		State[GetInputCode(cBUTTON_X)] = 0.f;
		State[GetInputCode(cBUTTON_Y)] = 0.f;
		State[GetInputCode(cBUTTON_BACK)] = 0.f;
		State[GetInputCode(cBUTTON_GUIDE)] = 0.f;
		State[GetInputCode(cBUTTON_START)] = 0.f;
		State[GetInputCode(cBUTTON_LEFTSTICK)] = 0.f;
		State[GetInputCode(cBUTTON_RIGHTSTICK)] = 0.f;
		State[GetInputCode(cBUTTON_LEFTSHOULDER)] = 0.f;
		State[GetInputCode(cBUTTON_RIGHTSHOULDER)] = 0.f;
		State[GetInputCode(cBUTTON_DPAD_UP)] = 0.f;
		State[GetInputCode(cBUTTON_DPAD_DOWN)] = 0.f;
		State[GetInputCode(cBUTTON_DPAD_LEFT)] = 0.f;
		State[GetInputCode(cBUTTON_DPAD_RIGHT)] = 0.f;
		State[GetInputCode(cBUTTON_MISC1)] = 0.f;
		State[GetInputCode(cBUTTON_PADDLE1)] = 0.f;
		State[GetInputCode(cBUTTON_PADDLE2)] = 0.f;
		State[GetInputCode(cBUTTON_PADDLE3)] = 0.f;
		State[GetInputCode(cBUTTON_PADDLE4)] = 0.f;
		State[GetInputCode(cBUTTON_TOUCHPAD)] = 0.f;
		State[GetInputCode(cAXIS_LEFTX)] = 0.f;
		State[GetInputCode(cAXIS_LEFTY)] = 0.f;
		State[GetInputCode(cAXIS_RIGHTX)] = 0.f;
		State[GetInputCode(cAXIS_RIGHTY)] = 0.f;
		State[GetInputCode(cAXIS_TRIGGERLEFT)] = 0.f;
		State[GetInputCode(cAXIS_TRIGGERRIGHT)] = 0.f;
	}

	wContextImpl(InputContext);

	float GetButton(const InputName& button)
	{
		return GetAxis(button).x;
	}

	vec2 GetAxisNoRecurse(const InputName& axis)
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

			out = itr->second.limitToUnit ? limit(out, 1.f) : out;
		}

		return out;
	}

	vec2 GetAxis(const InputName& axis)
	{
		vec2 out = GetAxisNoRecurse(axis);

		auto vitr = ctx->VirtualAxes.find(axis);
		if (vitr != ctx->VirtualAxes.end())
		{
			for (const auto& child : vitr->second.axes)
			{
				out += GetAxisNoRecurse(child);
			}

			out = vitr->second.limitToUnit ? limit(out, 1.f) : out;
		}

		return out;
	}

	void CreateAxis(const InputName& name)
	{
		if (AxisExists(name))
		{
			log_app("w~Axis already exists. %s", name);
			return;
		}

		ctx->Axes.emplace(name, InputAxis{});
	}

	void CreateVirtualAxis(const InputName& name)
	{
		if (VirtualAxisExists(name))
		{
			log_app("w~Virtual axis already exists. %s", name);
			return;
		}

		ctx->VirtualAxes.emplace(name, VirtualAxis{});
	}

	bool AxisExists(const InputName& axis)
	{
		return ctx->Axes.find(axis) != ctx->Axes.end();
	}

	bool VirtualAxisExists(const InputName& axis)
	{
		return ctx->VirtualAxes.find(axis) != ctx->VirtualAxes.end();
	}

	void SetDeadzone(const InputName& axis, float deadzone)
	{
		if (!AxisExists(axis))
		{
			log_app("w~Axis doesn't exist. %s", axis);
			return;
		}

		ctx->Axes[axis].deadzone = deadzone;
	}

	void SetAxisComponent(const InputName& axis, int code, vec2 weight)
	{
		if (!AxisExists(axis))
		{
			log_app("w~Axis doesn't exist. %s", axis);
			return;
		}

		ctx->Axes[axis].components.emplace(code, weight);
		ctx->Mapping.emplace(code, axis);
	}

	void SetAxisComponent(const InputName& axis, KeyboardInput scancode, vec2 weight)
	{
		SetAxisComponent(axis, GetInputCode(scancode), weight);
	}

	void SetAxisComponent(const InputName& axis, ControllerInput input, vec2 weight)
	{
		SetAxisComponent(axis, GetInputCode(input), weight);
	}

	void SetVirtualAxisComponent(const InputName& axis, const InputName& component)
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

		ctx->VirtualAxes[axis].axes.push_back(component);

		for (const auto& c : ctx->Axes[component].components) // we need to update mapping 
		{
			ctx->Mapping[c.first] = axis;
		}
	}

	InputName empty;

	const InputName& GetMapping(int code)
	{
		auto itr = ctx->Mapping.find(code);
		return itr != ctx->Mapping.end() ? itr->second : empty;
	}

	const InputName& GetMapping(KeyboardInput scancode)
	{
		return GetMapping(GetInputCode(scancode));
	}

	const InputName& GetMapping(ControllerInput scancode)
	{
		return GetMapping(GetInputCode(scancode));
	}

	// internal

	void SetState(int code, float state)
	{
		//if (code < 0 || code >= NUMBER_OF_STATES)
		//{
		//	log_app("e~Tried to set state of code that is invalid. %d -> %f", code, state);
		//	return;
		//}

		// grows forever if new codes are put in

		ctx->State[code] = state;
	}

    void WriteInputPack(const std::string& filepath)
    {
        log_io("Saving input pack %s", filepath.c_str());

        std::ofstream out(filepath);
        if (out.is_open()) json_writer(out).write(*GetContext());
    }

    void ReadInputPack(const std::string& filepath)
    {
        log_io("Loading input pack %s", filepath.c_str());

        std::ifstream in(filepath);
        if (in.is_open()) json_reader(in).read(*GetContext());
    }

	void SetViewportBounds(vec2 screenMin, vec2 screenSize)
	{
		ctx->ViewportMin = screenMin;
		ctx->ViewportSize = screenSize;
	}

	vec2 MapToViewport(float screenX, float screenY)
	{
		return clamp((vec2(screenX, screenY) - ctx->ViewportMin) / ctx->ViewportSize, vec2(-1.f), vec2(1.f));
	}
}