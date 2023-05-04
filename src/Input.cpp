#include "Input.h"
#include "Log.h"

InputAxisBuilder InputMap::CreateAxis(const InputName& name)
{
    InputAxis* axis = nullptr;
        
    if (_AxisExists(name))
        log_app("w~Axis already exists. %s", name.c_str());
    else
        axis = &Axes.emplace(name, InputAxis{}).first->second;
        
    return InputAxisBuilder(this, axis, name);
}

AxisGroupBuilder InputMap::CreateGroupAxis(const InputName& name)
{
    AxisGroup* group = nullptr;
        
    if (_GroupAxisExists(name))
        log_app("w~Group axis already exists. %s", name.c_str());
    else
        group = &GroupAxes.emplace(name, AxisGroup{}).first->second;
        
    return AxisGroupBuilder(this, group, name);
}

vec2 InputMap::GetAxis(const InputName& axis)
{
	if (!_GroupAxisExists(axis))
		return _GetAxisNoRecurse(axis);

	const AxisGroup& group = GroupAxes.at(axis);

	if (_FailsMask(group.settings.mask))
		return vec2(0.f);

	vec2 sum = vec2(0.f);

	for (const InputName& name : group.axes)
		sum += _GetAxisNoRecurse(name);

	if (length(sum) < group.settings.deadzone)
		return vec2(0.f);

	if (group.settings.normalized)
		sum = safe_normalize(sum);

	if (group.settings.limitToUnit)
		sum = limit(sum, 1.f);

	return sum;
}

float InputMap::GetButton(const InputName& button)
{
    return GetAxis(button).x;
}

bool InputMap::Once(const InputName& button)
{
	bool down = GetButton(button) > 0.f;

	if (!down) // If button isn't down, clear state
	{
		OnceButtonIsDown.erase(button);
		return false;
	}

	// Button is down

	if (OnceButtonIsDown.count(button) != 0) // If was already down
		return false;

	OnceButtonIsDown.insert(button);

	return true;
}

void InputMap::SetActiveMask(const std::string& mask)
{
    activeMask = mask;
}

void InputMap::SetViewportBounds(vec2 screenMin, vec2 screenSize)
{
    ViewportMin = screenMin;
    ViewportSize = screenSize;
}

vec2 InputMap::MapToViewport(float screenX, float screenY)
{
    return clamp((vec2(screenX, screenY) - ViewportMin) / ViewportSize, vec2(-1.f), vec2(1.f));
}

void InputMap::UpdateStates(float deltaTime)
{
    // I want this to actually calculate falloff base on a setting
    // but only need it for mouse velocity right now, and that should have immediate falloff

    State[GetInputCode(MOUSE_VEL_X)] = 0;
    State[GetInputCode(MOUSE_VEL_Y)] = 0;
    State[GetInputCode(MOUSE_VEL_WHEEL_X)] = 0;
    State[GetInputCode(MOUSE_VEL_WHEEL_Y)] = 0;
}

bool InputMap::_FailsMask(const std::string& mask)
{
	return mask.size() != 0
		&& mask != activeMask;
}

bool InputMap::_AxisExists(const InputName& axis)
{
    return Axes.find(axis) != Axes.end();
}

bool InputMap::_GroupAxisExists(const InputName& axis)
{
    return GroupAxes.find(axis) != GroupAxes.end();
}

vec2 InputMap::_GetAxisNoRecurse(const InputName& axisName)
{
	if (!_AxisExists(axisName))
		return vec2(0.f);

	const InputAxis& axis = Axes.at(axisName);

	if (_FailsMask(axis.settings.mask))
		return vec2(0.f);

	vec2 sum = vec2(0.f);

	for (const auto& [code, component] : axis.components)
		sum += State[code] * component;

	if (length(sum) < axis.settings.deadzone)
		return vec2(0.f);

	if (axis.settings.normalized)
		sum = safe_normalize(sum);

	if (axis.settings.limitToUnit)
		sum = limit(sum, 1.f);

	return sum;
}

InputAxisBuilder::InputAxisBuilder(InputMap* map, InputAxis* axis, const InputName& name)
    : InputAxisSettingsBuilder (&axis->settings)
    , m_map                    (map)
    , m_axis                   (axis)
    , m_name                   (name)
{}

InputAxisBuilder& InputAxisBuilder::Map(InputCode code, vec2 weight)
{
    m_axis->components.emplace(code, weight);
    m_map->Mapping.emplace(code, m_name);
    return *this;
}

InputAxisBuilder& InputAxisBuilder::MapButton(InputCode code, float weight)
{
    return Map(code, vec2(weight, 0.f));
}

AxisGroupBuilder::AxisGroupBuilder(InputMap* map, AxisGroup* axis, const InputName& name)
    : InputAxisSettingsBuilder (&axis->settings)
    , m_map                    (map)
    , m_axis                   (axis)
    , m_name                   (name)
{}

AxisGroupBuilder& AxisGroupBuilder::Map(const InputName& name)
{
    m_axis->axes.push_back(name);

    // Update mappings - This is not so good
        
    for (const auto& c : m_map->Axes[name].components)
        m_map->Mapping[c.first] = m_name;
        
    return *this;
}

// internal

void InputMap::SetState(int code, float state)
{
	// all valid states get registered when the context is created,
	// so check here to stop map from growing
	if (State.count(code) == 0)
	{
		log_app("w~Tried to set state of invalid input code. %d -> %f", code, state);
		return;
	}

	State[code] = state;
}

const InputName& InputMap::GetMapping(InputCode code)
{
    static InputName _default;
        
    auto itr = Mapping.find(code);
    return itr != Mapping.end() ? itr->second : _default;
}

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

InputMap::InputMap()
{
    ViewportMin = vec2(0, 0);
    ViewportSize = vec2(1, 1);

    // populate initial states, kinda sucks to have to do this
    // but needed for iterating available states by using the keys of State

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
