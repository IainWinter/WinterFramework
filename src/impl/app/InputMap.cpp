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

		meta::describe<InputAxisSettings>()
			.name("InputAxisSettings")
			.member<&InputAxisSettings::deadzone>("deadzone")
			.member<&InputAxisSettings::limitToUnit>("limitToUnit")
			.member<&InputAxisSettings::normalized>("normalized")
			.member<&InputAxisSettings::mask>("mask");

		meta::describe<InputAxis>()
			.name("InputAxis")
			.member<&InputAxis::settings>("settings")
			.member<&InputAxis::components>("components");

		meta::describe<AxisGroup>()
			.name("AxisGroup")
			.member<&AxisGroup::settings>("settings")
			.member<&AxisGroup::axes>("axes");

		meta::describe<InputContext>()
			.name("InputContext")
			.member<&InputContext::GroupAxes>("GroupAxes")
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

	bool _FailsMask(const std::string& mask)
	{
		return mask.size() != 0
			&& mask != ctx->activeMask;
	}

	vec2 _GetAxisNoRecurse(const InputName& axisName)
	{
		if (ctx->Axes.count(axisName) == 0)
		{
			return vec2(0.f);
		}

		const InputAxis& axis = ctx->Axes.at(axisName);

		if (_FailsMask(axis.settings.mask))
		{
			return vec2(0.f);
		}

		vec2 sum = vec2(0.f);

		for (const auto& [code, component] : axis.components)
		{
			sum += ctx->State[code] * component;
		}

		if (length(sum) < axis.settings.deadzone)
		{
			return vec2(0.f);
		}

		if (axis.settings.normalized)
		{
			sum = safe_normalize(sum);
		}

		if (axis.settings.limitToUnit)
		{
			sum = limit(sum, 1.f);
		}

		return sum;
	}

	vec2 GetAxis(const InputName& axis)
	{
		if (ctx->GroupAxes.count(axis) == 0)
		{
			return _GetAxisNoRecurse(axis);
		}

		const AxisGroup& group = ctx->GroupAxes.at(axis);

		if (_FailsMask(group.settings.mask))
		{
			return vec2(0.f);
		}

		vec2 sum = vec2(0.f);

		for (const InputName& name : group.axes)
		{
			sum += _GetAxisNoRecurse(name);
		}

		if (length(sum) < group.settings.deadzone)
		{
			return vec2(0.f);
		}

		if (group.settings.normalized)
		{
			sum = safe_normalize(sum);
		}

		if (group.settings.limitToUnit)
		{
			sum = limit(sum, 1.f);
		}

		return sum;
	}

	bool Once(const InputName& button)
	{
		bool down = GetButton(button) > 0.f;

		if (!down) // If button isn't down, clear state
		{
			ctx->OnceButtonIsDown.erase(button);
			return false;
		}

		// Button is down

		if (ctx->OnceButtonIsDown.count(button) != 0) // If was already down
		{
			return false;
		}

		ctx->OnceButtonIsDown.insert(button);

		return true;
	}

	bool AxisExists(const InputName& axis)
	{
		return ctx->Axes.find(axis) != ctx->Axes.end();
	}

	bool GroupAxisExists(const InputName& axis)
	{
		return ctx->GroupAxes.find(axis) != ctx->GroupAxes.end();
	}

	InputAxis& CreateAxis(const InputName& name)
	{
		if (AxisExists(name))
		{
			log_app("w~Axis already exists. %s", name.c_str());

			static InputAxis _default;
			return _default;
		}

		return ctx->Axes.emplace(name, InputAxis{}).first->second;
	}

	AxisGroup& CreateGroupAxis(const InputName& name)
	{
		if (GroupAxisExists(name))
		{
			log_app("w~Group axis already exists. %s", name.c_str());

			static AxisGroup _default;
			return _default;
		}

		return ctx->GroupAxes.emplace(name, AxisGroup{}).first->second;
	}

	void SetAxisSettings(const InputName& axis, const InputAxisSettings& settings)
	{
		if (!AxisExists(axis))
		{
			log_app("w~Axis doesn't exist. %s", axis.c_str());
			return;
		}

		ctx->Axes.at(axis).settings = settings;
	}

	void SetGroupAxisSettings(const InputName& axis, const InputAxisSettings& settings)
	{
		if (!GroupAxisExists(axis))
		{
			log_app("w~Group axis doesn't exist. %s", axis.c_str());
			return;
		}

		ctx->GroupAxes.at(axis).settings = settings;
	}

	void SetAxisComponent(const InputName& axis, InputCode code, vec2 weight)
	{
		if (!AxisExists(axis))
		{
			log_app("w~Axis doesn't exist. %s", axis.c_str());
			return;
		}

		ctx->Axes[axis].components.emplace(code, weight);
		ctx->Mapping.emplace(code, axis);
	}

	void SetGroupAxisComponent(const InputName& axis, const InputName& component)
	{
		if (!GroupAxisExists(axis))
		{
			log_app("w~Group axis doesn't exist. %s", axis.c_str());
			return;
		}

		if (!AxisExists(component))
		{
			log_app("w~Axis doesn't exist. %s", axis.c_str());
			return;
		}

		ctx->GroupAxes[axis].axes.push_back(component);

		for (const auto& c : ctx->Axes[component].components) // we need to update mapping 
		{
			ctx->Mapping[c.first] = axis;
		}
	}

	InputName empty;

	const InputName& GetMapping(InputCode code)
	{
		auto itr = ctx->Mapping.find(code);
		return itr != ctx->Mapping.end() ? itr->second : empty;
	}

	// internal

	void SetState(int code, float state)
	{
		// all valid states get registered when the context is created,
		// so check here to stop map from growing
		if (ctx->State.count(code) == 0)
		{
			log_app("w~Tried to set state of invalid input code. %d -> %f", code, state);
			return;
		}

		ctx->State[code] = state;
	}

    void WriteInputPack(const std::string& filepath)
    {
        log_io("Saving input pack %s", filepath.c_str());

        std::ofstream out(filepath);
        if (out.is_open()) json_writer(out).write(*ctx.GetCurrent());
    }

    void ReadInputPack(const std::string& filepath)
    {
        log_io("Loading input pack %s", filepath.c_str());

        std::ifstream in(filepath);
        if (in.is_open()) json_reader(in).read(*ctx.GetCurrent());
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

	void SetActiveMask(const std::string& mask)
	{
		ctx->activeMask = mask;
	}

	// comparisons

	bool InputAxisSettings::operator==(const InputAxisSettings& other) const
	{
		return deadzone    == other.deadzone
			&& limitToUnit == other.limitToUnit
			&& normalized  == other.normalized
			&& mask        == other.mask;
	}

	bool InputAxisSettings::operator!=(const InputAxisSettings& other) const
	{
		return !operator==(other);
	}

	bool InputAxis::operator==(const InputAxis& other) const
	{
		return std::equal(components.begin(), components.end(), other.components.begin())
			&& settings == other.settings;
	}

	bool InputAxis::operator!=(const InputAxis& other) const
	{
		return !operator==(other);
	}

	bool AxisGroup::operator==(const AxisGroup& other) const
	{
		return std::equal(axes.begin(), axes.end(), other.axes.begin())
			&& settings == other.settings;
	}

	bool AxisGroup::operator!=(const AxisGroup& other) const
	{
		return !operator==(other);
	}
}