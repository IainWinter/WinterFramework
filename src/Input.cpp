#include "Input.h"
#include "Log.h"

ControllerInput MapSDLGameControllerButton(SDL_GameControllerButton button)
{
	if (button == SDL_CONTROLLER_BUTTON_INVALID)
		return ControllerInput::cBUTTON_INVALID;

	return (ControllerInput)button;
}

ControllerInput MapSDLGameControllerAxis(SDL_GameControllerAxis axis)
{
	if (axis == SDL_CONTROLLER_AXIS_INVALID)
		return ControllerInput::cAXIS_INVALID;

	return (ControllerInput)(1 + axis + ControllerInput::cBUTTON_MAX);
}

int GetInputCode(KeyboardInput scancode)
{
	return (int)scancode;
}

int GetInputCode(MouseInput input)
{
	return (int)input + SDL_NUM_SCANCODES;
}

int GetInputCode(ControllerInput input)
{
	return (int)input + SDL_NUM_SCANCODES + MOUSE_INPUT_COUNT;
}

const char* GetInputCodeName(int code)
{
	static std::unordered_map<int, const char*> names =
	{
		{ GetInputCode(KEY_A),               "Key A" },
		{ GetInputCode(KEY_B),               "Key B" },
		{ GetInputCode(KEY_C),               "Key C" },
		{ GetInputCode(KEY_D),               "Key D" },
		{ GetInputCode(KEY_E),               "Key E" },
		{ GetInputCode(KEY_F),               "Key F" },
		{ GetInputCode(KEY_G),               "Key G" },
		{ GetInputCode(KEY_H),               "Key H" },
		{ GetInputCode(KEY_I),               "Key I" },
		{ GetInputCode(KEY_J),               "Key J" },
		{ GetInputCode(KEY_K),               "Key K" },
		{ GetInputCode(KEY_L),               "Key L" },
		{ GetInputCode(KEY_M),               "Key M" },
		{ GetInputCode(KEY_N),               "Key N" },
		{ GetInputCode(KEY_O),               "Key O" },
		{ GetInputCode(KEY_P),               "Key P" },
		{ GetInputCode(KEY_Q),               "Key Q" },
		{ GetInputCode(KEY_R),               "Key R" },
		{ GetInputCode(KEY_S),               "Key S" },
		{ GetInputCode(KEY_T),               "Key T" },
		{ GetInputCode(KEY_U),               "Key U" },
		{ GetInputCode(KEY_V),               "Key V" },
		{ GetInputCode(KEY_W),               "Key W" },
		{ GetInputCode(KEY_X),               "Key X" },
		{ GetInputCode(KEY_Y),               "Key Y" },
		{ GetInputCode(KEY_Z),               "Key Z" },
		{ GetInputCode(KEY_1),               "Key 1" },
		{ GetInputCode(KEY_2),               "Key 2" },
		{ GetInputCode(KEY_3),               "Key 3" },
		{ GetInputCode(KEY_4),               "Key 4" },
		{ GetInputCode(KEY_5),               "Key 5" },
		{ GetInputCode(KEY_6),               "Key 6" },
		{ GetInputCode(KEY_7),               "Key 7" },
		{ GetInputCode(KEY_8),               "Key 8" },
		{ GetInputCode(KEY_9),               "Key 9" },
		{ GetInputCode(KEY_0),               "Key 0" },
		{ GetInputCode(KEY_Return),          "Key Return" },
		{ GetInputCode(KEY_Escape),          "Key Escape" },
		{ GetInputCode(KEY_Backspace),       "Key Backspace" },
		{ GetInputCode(KEY_Tab),             "Key Tab" },
		{ GetInputCode(KEY_Space),           "Key Space" },
		{ GetInputCode(KEY_Minus),           "Key Minus" },
		{ GetInputCode(KEY_Equals),          "Key Equals" },
		{ GetInputCode(KEY_Bracket_Left),    "Key Bracket Left" },
		{ GetInputCode(KEY_Bracket_Right),   "Key Bracket Right" },
		{ GetInputCode(KEY_Backslash),       "Key Backslash" },
		{ GetInputCode(KEY_SemiColon),       "Key SemiColon" },
		{ GetInputCode(KEY_Apostrophe),      "Key Apostrophe" },
		{ GetInputCode(KEY_Grave),           "Key Grave" },
		{ GetInputCode(KEY_Comma),           "Key Comma" },
		{ GetInputCode(KEY_Period),          "Key Period" },
		{ GetInputCode(KEY_Slash),           "Key Slash" },
		{ GetInputCode(KEY_Control_Left),    "Key Control Left" },
		{ GetInputCode(KEY_Shift_Left),      "Key Shift Left" },
		{ GetInputCode(KEY_Alt_Left),        "Key Alt Left" },
		{ GetInputCode(KEY_GUI_Left),        "Key GUI Left" },
		{ GetInputCode(KEY_Control_Right),   "Key Control Right" },
		{ GetInputCode(KEY_Shift_Right),     "Key Shift Right" },
		{ GetInputCode(KEY_Alt_Right),       "Key Alt Right" },
		{ GetInputCode(KEY_GUI_Right),       "Key GUI Right" },
		{ GetInputCode(KEY_Right),           "Key Right" },
		{ GetInputCode(KEY_Left),            "Key Left" },
		{ GetInputCode(KEY_Down),            "Key Down" },
		{ GetInputCode(KEY_Up),              "Key Up" },
		{ GetInputCode(KEY_Lock_Caps),       "Key Lock Caps" },
		{ GetInputCode(KEY_Lock_Scroll),     "Key Lock Scroll" },
		{ GetInputCode(KEY_Lock_Number),     "Key Lock Number" },
		{ GetInputCode(KEY_PrintScreen),     "Key Print Screen" },
		{ GetInputCode(KEY_Pause),           "Key Pause" },
		{ GetInputCode(KEY_Insert),          "Key Insert" },
		{ GetInputCode(KEY_Home),            "Key Home" },
		{ GetInputCode(KEY_Page_Up),         "Key Page_Up" },
		{ GetInputCode(KEY_Page_Down),       "Key Page_Down" },
		{ GetInputCode(KEY_Delete),          "Key Delete" },
		{ GetInputCode(KEY_End),             "Key End" },
		{ GetInputCode(KEY_Keypad_Divide),   "Key Keypad Divide" },
		{ GetInputCode(KEY_Keypad_Multiply), "Key Keypad Multiply" },
		{ GetInputCode(KEY_Keypad_Minus),    "Key Keypad Minus" },
		{ GetInputCode(KEY_Keypad_Plus),     "Key Keypad Plus" },
		{ GetInputCode(KEY_Keypad_Enter),    "Key Keypad Enter" },
		{ GetInputCode(KEY_Keypad_1),        "Key Keypad 1" },
		{ GetInputCode(KEY_Keypad_2),        "Key Keypad 2" },
		{ GetInputCode(KEY_Keypad_3),        "Key Keypad 3" },
		{ GetInputCode(KEY_Keypad_4),        "Key Keypad 4" },
		{ GetInputCode(KEY_Keypad_5),        "Key Keypad 5" },
		{ GetInputCode(KEY_Keypad_6),        "Key Keypad 6" },
		{ GetInputCode(KEY_Keypad_7),        "Key Keypad 7" },
		{ GetInputCode(KEY_Keypad_8),        "Key Keypad 8" },
		{ GetInputCode(KEY_Keypad_9),        "Key Keypad 9" },
		{ GetInputCode(KEY_Keypad_0),        "Key Keypad 0" },
		{ GetInputCode(KEY_Keypad_Period),   "Key Keypad Period" },
		{ GetInputCode(KEY_Keypad_Equals),   "Key Keypad Equals" },
		{ GetInputCode(KEY_Function_1),      "Key Function 1" },
		{ GetInputCode(KEY_Function_2),      "Key Function 2" },
		{ GetInputCode(KEY_Function_3),      "Key Function 3" },
		{ GetInputCode(KEY_Function_4),      "Key Function 4" },
		{ GetInputCode(KEY_Function_5),      "Key Function 5" },
		{ GetInputCode(KEY_Function_6),      "Key Function 6" },
		{ GetInputCode(KEY_Function_7),      "Key Function 7" },
		{ GetInputCode(KEY_Function_8),      "Key Function 8" },
		{ GetInputCode(KEY_Function_9),      "Key Function 9" },
		{ GetInputCode(KEY_Function_10),     "Key Function 10" },
		{ GetInputCode(KEY_Function_11),     "Key Function 11" },
		{ GetInputCode(KEY_Function_12),     "Key Function 12" },
		{ GetInputCode(KEY_Function_13),     "Key Function 13" },
		{ GetInputCode(KEY_Function_14),     "Key Function 14" },
		{ GetInputCode(KEY_Function_15),     "Key Function 15" },
		{ GetInputCode(KEY_Function_16),     "Key Function 16" },
		{ GetInputCode(KEY_Function_17),     "Key Function 17" },
		{ GetInputCode(KEY_Function_18),     "Key Function 18" },
		{ GetInputCode(KEY_Function_19),     "Key Function 19" },
		{ GetInputCode(KEY_Function_20),     "Key Function 20" },
		{ GetInputCode(KEY_Function_21),     "Key Function 21" },
		{ GetInputCode(KEY_Function_22),     "Key Function 22" },
		{ GetInputCode(KEY_Function_23),     "Key Function 23" },
		{ GetInputCode(KEY_Function_24),     "Key Function 24" },

		{ GetInputCode(MOUSE_LEFT),        "Mouse Left" },
		{ GetInputCode(MOUSE_MIDDLE),      "Mouse Middle" },
		{ GetInputCode(MOUSE_RIGHT),       "Mouse Right" },
		{ GetInputCode(MOUSE_X1),          "Mouse X1" },
		{ GetInputCode(MOUSE_X2),          "Mouse X2" },
		{ GetInputCode(MOUSE_POS_X),       "Mouse Position X" },
		{ GetInputCode(MOUSE_POS_Y),       "Mouse Position Y" },
		{ GetInputCode(MOUSE_VEL_X),       "Mouse Velocity X" },
		{ GetInputCode(MOUSE_VEL_Y),       "Mouse Velocity Y" },
		{ GetInputCode(MOUSE_VEL_WHEEL_X), "Mouse Wheel Velocity X" },
		{ GetInputCode(MOUSE_VEL_WHEEL_Y), "Mouse Wheel Velocity Y" },

		{ GetInputCode(cBUTTON_A),             "Button A" },
		{ GetInputCode(cBUTTON_B),             "Button B" },
		{ GetInputCode(cBUTTON_X),             "Button X" },
		{ GetInputCode(cBUTTON_Y),             "Button Y" },
		{ GetInputCode(cBUTTON_BACK),          "Button BACK" },
		{ GetInputCode(cBUTTON_GUIDE),         "Button GUIDE" },
		{ GetInputCode(cBUTTON_START),         "Button START" },
		{ GetInputCode(cBUTTON_LEFTSTICK),     "Button Left Stick" },
		{ GetInputCode(cBUTTON_RIGHTSTICK),    "Button Right Stick" },
		{ GetInputCode(cBUTTON_LEFTSHOULDER),  "Button Left Shoulder" },
		{ GetInputCode(cBUTTON_RIGHTSHOULDER), "Button Right Shoulder" },
		{ GetInputCode(cBUTTON_DPAD_UP),       "Button DPAD Up" },
		{ GetInputCode(cBUTTON_DPAD_DOWN),     "Button DPAD Down" },
		{ GetInputCode(cBUTTON_DPAD_LEFT),     "Button DPAD Left" },
		{ GetInputCode(cBUTTON_DPAD_RIGHT),    "Button DPAD Right" },
		{ GetInputCode(cBUTTON_MISC1),         "Button Misc 1" },
		{ GetInputCode(cBUTTON_PADDLE1),       "Button Paddle 1" },
		{ GetInputCode(cBUTTON_PADDLE2),       "Button Paddle 2" },
		{ GetInputCode(cBUTTON_PADDLE3),       "Button Paddle 3" },
		{ GetInputCode(cBUTTON_PADDLE4),       "Button Paddle 4" },
		{ GetInputCode(cBUTTON_TOUCHPAD),      "Button TOUCHPAD" },
		{ GetInputCode(cAXIS_LEFTX),           "Axis Left X" },
		{ GetInputCode(cAXIS_LEFTY),           "Axis Left Y" },
		{ GetInputCode(cAXIS_RIGHTX),          "Axis Right X" },
		{ GetInputCode(cAXIS_RIGHTY),          "Axis Right Y" },
		{ GetInputCode(cAXIS_TRIGGERLEFT),     "Axis Trigger Left" },
		{ GetInputCode(cAXIS_TRIGGERRIGHT),    "Axis Trigger Right" }
	};

	auto itr = names.find(code);

	if (itr == names.end())
	{
        return ".";
	}

    return itr->second;
}

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
    
    lastInputWasController = GetInputCode(ControllerInput::cBUTTON_A) <= code
                          && GetInputCode(ControllerInput::cAXIS_MAX) > code;

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
