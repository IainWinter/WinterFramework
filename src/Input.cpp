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
		return _GetAxisNoRecurse(axis, false);

	const AxisGroup& group = GroupAxes.at(axis);

	if (_FailsMask(group.settings.mask))
		return vec2(0.f);

	vec2 sum = vec2(0.f);

    if (group.settings.useOnlyLatestInput)
    {
        InputName lastStateNameUsed = {};
        int lastFrameUsed = 0;

        for (const InputName& name : group.axes)
        {
            for (const auto& [code, component] : Axes.at(name).components)
            {
                InputState& state = State.at(code);
                if (state.frameLastUpdated > lastFrameUsed)
                {
                    lastStateNameUsed = name;
                    lastFrameUsed = state.frameLastUpdated;
                }
            }
        }

        sum += _GetAxisNoRecurse(lastStateNameUsed, group.settings.useOnlyInputSetThisFrame);
    }

    else
    {
        for (const InputName& name : group.axes)
            sum += _GetAxisNoRecurse(name, group.settings.useOnlyInputSetThisFrame);
    }

    sum = _ApplySettings(sum, group.settings);

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

bool InputMap::IsUsingController() const
{
    return lastInputWasController;
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

    State[GetInputCode(MOUSE_VEL_X)].value = 0;
    State[GetInputCode(MOUSE_VEL_Y)].value = 0;
    State[GetInputCode(MOUSE_VEL_WHEEL_X)].value = 0;
    State[GetInputCode(MOUSE_VEL_WHEEL_Y)].value = 0;
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

vec2 InputMap::_GetAxisNoRecurse(const InputName& axisName, bool useOnlyInputSetThisFrameOverride)
{
	if (!_AxisExists(axisName))
		return vec2(0.f);

	const InputAxis& axis = Axes.at(axisName);

	if (_FailsMask(axis.settings.mask))
		return vec2(0.f);

    bool useOnlyInputSetThisFrame = axis.settings.useOnlyInputSetThisFrame || useOnlyInputSetThisFrameOverride;

	vec2 sum = vec2(0.f);

    if (axis.settings.useOnlyLatestInput)
    {
        vec2 lastStateValueUsed = {};
        int lastFrameUsed = 0;

        for (const auto& [code, component] : axis.components)
        {
            InputState& state = State.at(code);

            if (state.frameLastUpdated > lastFrameUsed)
            {
                lastStateValueUsed = state.value * component;
                lastFrameUsed = state.frameLastUpdated;
            }
        }

        if (useOnlyInputSetThisFrame && lastFrameUsed != activeFrame)
            return vec2(0.f);

        sum += lastStateValueUsed;
    }

    else
    {
        for (const auto& [code, component] : axis.components)
        {
            InputState& state = State[code];

            if (useOnlyInputSetThisFrame && state.frameLastUpdated != activeFrame)
                continue;

		    sum += state.value * component;
        }
    }

    sum = _ApplySettings(sum, axis.settings);

	return sum;
}

vec2 InputMap::_ApplySettings(vec2 in, const InputAxisSettings& settings)
{
    if (length(in) < settings.deadzone)
        return vec2(0.f);

    if (settings.normalized)
        in = safe_normalize(in);

    if (settings.limitToUnit)
        in = limit(in, 1.f);

    for (const auto& process : settings.userPipeline)
        in = process(in);

    return in;
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
    m_map->Mapping[code].insert(m_name);
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

    for (const auto& c : m_map->Axes[name].components)
        m_map->Mapping[c.first].insert(m_name);
        
    return *this;
}

// internal

void InputMap::SetActiveFrame(int frame)
{
    activeFrame = frame;
}

void InputMap::SetState(int code, float state)
{
	// all valid states get registered when the context is created,
	// so check here to stop map from growing
	if (State.count(code) == 0)
	{
		log_app("w~Tried to set state of invalid input code. %d -> %f", code, state);
		return;
	}
    
    lastInputWasController = GetInputCode(ControllerInput::cBUTTON_A) >= code
                          && GetInputCode(ControllerInput::cAXIS_MAX) < code;

    State[code] = { state, activeFrame };
}

const std::unordered_set<InputName>& InputMap::GetMapping(InputCode code)
{
    static std::unordered_set<InputName> _default;
        
    auto itr = Mapping.find(code);
    return itr != Mapping.end() ? itr->second : _default;
}

float InputMap::GetRawState(InputCode code)
{
    auto itr = State.find(code);
    return itr != State.end() ? itr->second.value : 0.f;
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

    activeMask = "";
    activeFrame = 0;

    // populate initial states, kinda sucks to have to do this
    // but needed for iterating available states by using the keys of State

    State[GetInputCode(KEY_A)].value = 0.f;
    State[GetInputCode(KEY_B)].value = 0.f;
    State[GetInputCode(KEY_C)].value = 0.f;
    State[GetInputCode(KEY_D)].value = 0.f;
    State[GetInputCode(KEY_E)].value = 0.f;
    State[GetInputCode(KEY_F)].value = 0.f;
    State[GetInputCode(KEY_G)].value = 0.f;
    State[GetInputCode(KEY_H)].value = 0.f;
    State[GetInputCode(KEY_I)].value = 0.f;
    State[GetInputCode(KEY_J)].value = 0.f;
    State[GetInputCode(KEY_K)].value = 0.f;
    State[GetInputCode(KEY_L)].value = 0.f;
    State[GetInputCode(KEY_M)].value = 0.f;
    State[GetInputCode(KEY_N)].value = 0.f;
    State[GetInputCode(KEY_O)].value = 0.f;
    State[GetInputCode(KEY_P)].value = 0.f;
    State[GetInputCode(KEY_Q)].value = 0.f;
    State[GetInputCode(KEY_R)].value = 0.f;
    State[GetInputCode(KEY_S)].value = 0.f;
    State[GetInputCode(KEY_T)].value = 0.f;
    State[GetInputCode(KEY_U)].value = 0.f;
    State[GetInputCode(KEY_V)].value = 0.f;
    State[GetInputCode(KEY_W)].value = 0.f;
    State[GetInputCode(KEY_X)].value = 0.f;
    State[GetInputCode(KEY_Y)].value = 0.f;
    State[GetInputCode(KEY_Z)].value = 0.f;
    State[GetInputCode(KEY_1)].value = 0.f;
    State[GetInputCode(KEY_2)].value = 0.f;
    State[GetInputCode(KEY_3)].value = 0.f;
    State[GetInputCode(KEY_4)].value = 0.f;
    State[GetInputCode(KEY_5)].value = 0.f;
    State[GetInputCode(KEY_6)].value = 0.f;
    State[GetInputCode(KEY_7)].value = 0.f;
    State[GetInputCode(KEY_8)].value = 0.f;
    State[GetInputCode(KEY_9)].value = 0.f;
    State[GetInputCode(KEY_0)].value = 0.f;
    State[GetInputCode(KEY_Return)].value = 0.f;
    State[GetInputCode(KEY_Escape)].value = 0.f;
    State[GetInputCode(KEY_Backspace)].value = 0.f;
    State[GetInputCode(KEY_Tab)].value = 0.f;
    State[GetInputCode(KEY_Space)].value = 0.f;
    State[GetInputCode(KEY_Minus)].value = 0.f;
    State[GetInputCode(KEY_Equals)].value = 0.f;
    State[GetInputCode(KEY_Bracket_Left)].value = 0.f;
    State[GetInputCode(KEY_Bracket_Right)].value = 0.f;
    State[GetInputCode(KEY_Backslash)].value = 0.f;
    State[GetInputCode(KEY_SemiColon)].value = 0.f;
    State[GetInputCode(KEY_Apostrophe)].value = 0.f;
    State[GetInputCode(KEY_Grave)].value = 0.f;
    State[GetInputCode(KEY_Comma)].value = 0.f;
    State[GetInputCode(KEY_Period)].value = 0.f;
    State[GetInputCode(KEY_Slash)].value = 0.f;
    State[GetInputCode(KEY_Control_Left)].value = 0.f;
    State[GetInputCode(KEY_Shift_Left)].value = 0.f;
    State[GetInputCode(KEY_Alt_Left)].value = 0.f;
    State[GetInputCode(KEY_GUI_Left)].value = 0.f;
    State[GetInputCode(KEY_Control_Right)].value = 0.f;
    State[GetInputCode(KEY_Shift_Right)].value = 0.f;
    State[GetInputCode(KEY_Alt_Right)].value = 0.f;
    State[GetInputCode(KEY_GUI_Right)].value = 0.f;
    State[GetInputCode(KEY_Right)].value = 0.f;
    State[GetInputCode(KEY_Left)].value = 0.f;
    State[GetInputCode(KEY_Down)].value = 0.f;
    State[GetInputCode(KEY_Up)].value = 0.f;
    State[GetInputCode(KEY_Lock_Caps)].value = 0.f;
    State[GetInputCode(KEY_Lock_Scroll)].value = 0.f;
    State[GetInputCode(KEY_Lock_Number)].value = 0.f;
    State[GetInputCode(KEY_PrintScreen)].value = 0.f;
    State[GetInputCode(KEY_Pause)].value = 0.f;
    State[GetInputCode(KEY_Insert)].value = 0.f;
    State[GetInputCode(KEY_Home)].value = 0.f;
    State[GetInputCode(KEY_Page_Up)].value = 0.f;
    State[GetInputCode(KEY_Page_Down)].value = 0.f;
    State[GetInputCode(KEY_Delete)].value = 0.f;
    State[GetInputCode(KEY_End)].value = 0.f;
    State[GetInputCode(KEY_Keypad_Divide)].value = 0.f;
    State[GetInputCode(KEY_Keypad_Multiply)].value = 0.f;
    State[GetInputCode(KEY_Keypad_Minus)].value = 0.f;
    State[GetInputCode(KEY_Keypad_Plus)].value = 0.f;
    State[GetInputCode(KEY_Keypad_Enter)].value = 0.f;
    State[GetInputCode(KEY_Keypad_1)].value = 0.f;
    State[GetInputCode(KEY_Keypad_2)].value = 0.f;
    State[GetInputCode(KEY_Keypad_3)].value = 0.f;
    State[GetInputCode(KEY_Keypad_4)].value = 0.f;
    State[GetInputCode(KEY_Keypad_5)].value = 0.f;
    State[GetInputCode(KEY_Keypad_6)].value = 0.f;
    State[GetInputCode(KEY_Keypad_7)].value = 0.f;
    State[GetInputCode(KEY_Keypad_8)].value = 0.f;
    State[GetInputCode(KEY_Keypad_9)].value = 0.f;
    State[GetInputCode(KEY_Keypad_0)].value = 0.f;
    State[GetInputCode(KEY_Keypad_Period)].value = 0.f;
    State[GetInputCode(KEY_Keypad_Equals)].value = 0.f;
    State[GetInputCode(KEY_Function_1)].value = 0.f;
    State[GetInputCode(KEY_Function_2)].value = 0.f;
    State[GetInputCode(KEY_Function_3)].value = 0.f;
    State[GetInputCode(KEY_Function_4)].value = 0.f;
    State[GetInputCode(KEY_Function_5)].value = 0.f;
    State[GetInputCode(KEY_Function_6)].value = 0.f;
    State[GetInputCode(KEY_Function_7)].value = 0.f;
    State[GetInputCode(KEY_Function_8)].value = 0.f;
    State[GetInputCode(KEY_Function_9)].value = 0.f;
    State[GetInputCode(KEY_Function_10)].value = 0.f;
    State[GetInputCode(KEY_Function_11)].value = 0.f;
    State[GetInputCode(KEY_Function_12)].value = 0.f;
    State[GetInputCode(KEY_Function_13)].value = 0.f;
    State[GetInputCode(KEY_Function_14)].value = 0.f;
    State[GetInputCode(KEY_Function_15)].value = 0.f;
    State[GetInputCode(KEY_Function_16)].value = 0.f;
    State[GetInputCode(KEY_Function_17)].value = 0.f;
    State[GetInputCode(KEY_Function_18)].value = 0.f;
    State[GetInputCode(KEY_Function_19)].value = 0.f;
    State[GetInputCode(KEY_Function_20)].value = 0.f;
    State[GetInputCode(KEY_Function_21)].value = 0.f;
    State[GetInputCode(KEY_Function_22)].value = 0.f;
    State[GetInputCode(KEY_Function_23)].value = 0.f;
    State[GetInputCode(KEY_Function_24)].value = 0.f;

    State[GetInputCode(MOUSE_LEFT)].value = 0.f;
    State[GetInputCode(MOUSE_MIDDLE)].value = 0.f;
    State[GetInputCode(MOUSE_RIGHT)].value = 0.f;
    State[GetInputCode(MOUSE_X1)].value = 0.f;
    State[GetInputCode(MOUSE_X2)].value = 0.f;
    State[GetInputCode(MOUSE_POS_X)].value = 0.f;
    State[GetInputCode(MOUSE_POS_Y)].value = 0.f;
    State[GetInputCode(MOUSE_VEL_X)].value = 0.f;
    State[GetInputCode(MOUSE_VEL_Y)].value = 0.f;
    State[GetInputCode(MOUSE_VEL_WHEEL_X)].value = 0.f;
    State[GetInputCode(MOUSE_VEL_WHEEL_Y)].value = 0.f;

    State[GetInputCode(cBUTTON_A)].value = 0.f;
    State[GetInputCode(cBUTTON_B)].value = 0.f;
    State[GetInputCode(cBUTTON_X)].value = 0.f;
    State[GetInputCode(cBUTTON_Y)].value = 0.f;
    State[GetInputCode(cBUTTON_BACK)].value = 0.f;
    State[GetInputCode(cBUTTON_GUIDE)].value = 0.f;
    State[GetInputCode(cBUTTON_START)].value = 0.f;
    State[GetInputCode(cBUTTON_LEFTSTICK)].value = 0.f;
    State[GetInputCode(cBUTTON_RIGHTSTICK)].value = 0.f;
    State[GetInputCode(cBUTTON_LEFTSHOULDER)].value = 0.f;
    State[GetInputCode(cBUTTON_RIGHTSHOULDER)].value = 0.f;
    State[GetInputCode(cBUTTON_DPAD_UP)].value = 0.f;
    State[GetInputCode(cBUTTON_DPAD_DOWN)].value = 0.f;
    State[GetInputCode(cBUTTON_DPAD_LEFT)].value = 0.f;
    State[GetInputCode(cBUTTON_DPAD_RIGHT)].value = 0.f;
    State[GetInputCode(cBUTTON_MISC1)].value = 0.f;
    State[GetInputCode(cBUTTON_PADDLE1)].value = 0.f;
    State[GetInputCode(cBUTTON_PADDLE2)].value = 0.f;
    State[GetInputCode(cBUTTON_PADDLE3)].value = 0.f;
    State[GetInputCode(cBUTTON_PADDLE4)].value = 0.f;
    State[GetInputCode(cBUTTON_TOUCHPAD)].value = 0.f;
    State[GetInputCode(cAXIS_LEFTX)].value = 0.f;
    State[GetInputCode(cAXIS_LEFTY)].value = 0.f;
    State[GetInputCode(cAXIS_RIGHTX)].value = 0.f;
    State[GetInputCode(cAXIS_RIGHTY)].value = 0.f;
    State[GetInputCode(cAXIS_TRIGGERLEFT)].value = 0.f;
    State[GetInputCode(cAXIS_TRIGGERRIGHT)].value = 0.f;
}
