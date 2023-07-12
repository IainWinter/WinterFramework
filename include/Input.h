#pragma once

#include "SDL_scancode.h"
#include "SDL_gamecontroller.h"

#include "util/math.h"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>
#include <array>

/**
 * A type to identify axes, you should replace with an enum
 */
using InputName = std::string;

/**
 * A type to identify masks, you should replace with an enum
 */
using InputMask = std::string;

enum KeyboardInput
{
	KEY_INVALID          = 0,
	KEY_A                = SDL_SCANCODE_A,
	KEY_B                = SDL_SCANCODE_B,
	KEY_C                = SDL_SCANCODE_C,
	KEY_D                = SDL_SCANCODE_D,
	KEY_E                = SDL_SCANCODE_E,
	KEY_F                = SDL_SCANCODE_F,
	KEY_G                = SDL_SCANCODE_G,
	KEY_H                = SDL_SCANCODE_H,
	KEY_I                = SDL_SCANCODE_I,
	KEY_J                = SDL_SCANCODE_J,
	KEY_K                = SDL_SCANCODE_K,
	KEY_L                = SDL_SCANCODE_L,
	KEY_M                = SDL_SCANCODE_M,
	KEY_N                = SDL_SCANCODE_N,
	KEY_O                = SDL_SCANCODE_O,
	KEY_P                = SDL_SCANCODE_P,
	KEY_Q                = SDL_SCANCODE_Q,
	KEY_R                = SDL_SCANCODE_R,
	KEY_S                = SDL_SCANCODE_S,
	KEY_T                = SDL_SCANCODE_T,
	KEY_U                = SDL_SCANCODE_U,
	KEY_V                = SDL_SCANCODE_V,
	KEY_W                = SDL_SCANCODE_W,
	KEY_X                = SDL_SCANCODE_X,
	KEY_Y                = SDL_SCANCODE_Y,
	KEY_Z                = SDL_SCANCODE_Z,
	KEY_1                = SDL_SCANCODE_1,
	KEY_2                = SDL_SCANCODE_2,
	KEY_3                = SDL_SCANCODE_3,
	KEY_4                = SDL_SCANCODE_4,
	KEY_5                = SDL_SCANCODE_5,
	KEY_6                = SDL_SCANCODE_6,
	KEY_7                = SDL_SCANCODE_7,
	KEY_8                = SDL_SCANCODE_8,
	KEY_9                = SDL_SCANCODE_9,
	KEY_0                = SDL_SCANCODE_0,
	KEY_Return           = SDL_SCANCODE_RETURN,
	KEY_Escape           = SDL_SCANCODE_ESCAPE,
	KEY_Backspace        = SDL_SCANCODE_BACKSPACE,
	KEY_Tab              = SDL_SCANCODE_TAB,
	KEY_Space            = SDL_SCANCODE_SPACE,
	KEY_Minus            = SDL_SCANCODE_MINUS,
	KEY_Equals           = SDL_SCANCODE_EQUALS,
	KEY_Bracket_Left     = SDL_SCANCODE_LEFTBRACKET,
	KEY_Bracket_Right    = SDL_SCANCODE_RIGHTBRACKET,
	KEY_Backslash        = SDL_SCANCODE_BACKSLASH,
	KEY_SemiColon        = SDL_SCANCODE_SEMICOLON,
	KEY_Apostrophe       = SDL_SCANCODE_APOSTROPHE,
	KEY_Grave            = SDL_SCANCODE_GRAVE,
	KEY_Comma            = SDL_SCANCODE_COMMA,
	KEY_Period           = SDL_SCANCODE_PERIOD,
	KEY_Slash            = SDL_SCANCODE_SLASH,
	KEY_Control_Left     = SDL_SCANCODE_LCTRL,
	KEY_Shift_Left       = SDL_SCANCODE_LSHIFT,
	KEY_Alt_Left         = SDL_SCANCODE_LALT,
	KEY_GUI_Left         = SDL_SCANCODE_LGUI,
	KEY_Control_Right    = SDL_SCANCODE_RCTRL,
	KEY_Shift_Right      = SDL_SCANCODE_RSHIFT,
	KEY_Alt_Right        = SDL_SCANCODE_RALT,
	KEY_GUI_Right        = SDL_SCANCODE_RGUI,
	KEY_Right            = SDL_SCANCODE_RIGHT,
	KEY_Left             = SDL_SCANCODE_LEFT,
	KEY_Down             = SDL_SCANCODE_DOWN,
	KEY_Up               = SDL_SCANCODE_UP,
	KEY_Lock_Caps        = SDL_SCANCODE_CAPSLOCK,
	KEY_Lock_Scroll      = SDL_SCANCODE_SCROLLLOCK,
	KEY_Lock_Number      = SDL_SCANCODE_NUMLOCKCLEAR,
	KEY_PrintScreen      = SDL_SCANCODE_PRINTSCREEN,
	KEY_Pause            = SDL_SCANCODE_PAUSE,
	KEY_Insert           = SDL_SCANCODE_INSERT,
	KEY_Home             = SDL_SCANCODE_HOME,
	KEY_Page_Up          = SDL_SCANCODE_PAGEUP,
	KEY_Page_Down        = SDL_SCANCODE_PAGEDOWN,
	KEY_Delete           = SDL_SCANCODE_DELETE,
	KEY_End              = SDL_SCANCODE_END,
	KEY_Keypad_Divide    = SDL_SCANCODE_KP_DIVIDE,
	KEY_Keypad_Multiply  = SDL_SCANCODE_KP_MULTIPLY,
	KEY_Keypad_Minus     = SDL_SCANCODE_KP_MINUS,
	KEY_Keypad_Plus      = SDL_SCANCODE_KP_PLUS,
	KEY_Keypad_Enter     = SDL_SCANCODE_KP_ENTER,
	KEY_Keypad_1         = SDL_SCANCODE_KP_1,
	KEY_Keypad_2         = SDL_SCANCODE_KP_2,
	KEY_Keypad_3         = SDL_SCANCODE_KP_3,
	KEY_Keypad_4         = SDL_SCANCODE_KP_4,
	KEY_Keypad_5         = SDL_SCANCODE_KP_5,
	KEY_Keypad_6         = SDL_SCANCODE_KP_6,
	KEY_Keypad_7         = SDL_SCANCODE_KP_7,
	KEY_Keypad_8         = SDL_SCANCODE_KP_8,
	KEY_Keypad_9         = SDL_SCANCODE_KP_9,
	KEY_Keypad_0         = SDL_SCANCODE_KP_0,
	KEY_Keypad_Period    = SDL_SCANCODE_KP_PERIOD,
	KEY_Keypad_Equals    = SDL_SCANCODE_KP_EQUALS,
	KEY_Function_1       = SDL_SCANCODE_F1,
	KEY_Function_2       = SDL_SCANCODE_F2,
	KEY_Function_3       = SDL_SCANCODE_F3,
	KEY_Function_4       = SDL_SCANCODE_F4,
	KEY_Function_5       = SDL_SCANCODE_F5,
	KEY_Function_6       = SDL_SCANCODE_F6,
	KEY_Function_7       = SDL_SCANCODE_F7,
	KEY_Function_8       = SDL_SCANCODE_F8,
	KEY_Function_9       = SDL_SCANCODE_F9,
	KEY_Function_10      = SDL_SCANCODE_F10,
	KEY_Function_11      = SDL_SCANCODE_F11,
	KEY_Function_12      = SDL_SCANCODE_F12,
	KEY_Function_13      = SDL_SCANCODE_F13,
	KEY_Function_14      = SDL_SCANCODE_F14,
	KEY_Function_15      = SDL_SCANCODE_F15,
	KEY_Function_16      = SDL_SCANCODE_F16,
	KEY_Function_17      = SDL_SCANCODE_F17,
	KEY_Function_18      = SDL_SCANCODE_F18,
	KEY_Function_19      = SDL_SCANCODE_F19,
	KEY_Function_20      = SDL_SCANCODE_F20,
	KEY_Function_21      = SDL_SCANCODE_F21,
	KEY_Function_22      = SDL_SCANCODE_F22,
	KEY_Function_23      = SDL_SCANCODE_F23,
	KEY_Function_24      = SDL_SCANCODE_F24
};

enum MouseInput
{
	MOUSE_LEFT,
	MOUSE_MIDDLE,
	MOUSE_RIGHT,
	MOUSE_X1,
	MOUSE_X2,

	// these are in viewport space

	MOUSE_POS_X,
	MOUSE_POS_Y,

	MOUSE_VEL_X,
	MOUSE_VEL_Y,

	MOUSE_VEL_WHEEL_X,
	MOUSE_VEL_WHEEL_Y,

	MOUSE_INPUT_COUNT,

	// these are sent by the Window, EngineLoop handles breaking it into x,y/vx,vy
	MOUSE_VEL_POS,
	MOUSE_VEL_WHEEL,
};

// easier to combine the controller axis/buttons into a single enum
enum ControllerInput
{
	cAXIS_INVALID   = -10,
	cBUTTON_INVALID = -1,

    cBUTTON_A,
    cBUTTON_B,
    cBUTTON_X,
    cBUTTON_Y,
    cBUTTON_BACK,
    cBUTTON_GUIDE,
    cBUTTON_START,
    cBUTTON_LEFTSTICK,
    cBUTTON_RIGHTSTICK,
    cBUTTON_LEFTSHOULDER,
    cBUTTON_RIGHTSHOULDER,
    cBUTTON_DPAD_UP,
    cBUTTON_DPAD_DOWN,
    cBUTTON_DPAD_LEFT,
    cBUTTON_DPAD_RIGHT,
    cBUTTON_MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
    cBUTTON_PADDLE1,  /* Xbox Elite paddle P1 */
    cBUTTON_PADDLE2,  /* Xbox Elite paddle P3 */
    cBUTTON_PADDLE3,  /* Xbox Elite paddle P2 */
    cBUTTON_PADDLE4,  /* Xbox Elite paddle P4 */
    cBUTTON_TOUCHPAD, /* PS4/PS5 touchpad button */
    
	cBUTTON_MAX,

	cAXIS_LEFTX,
	cAXIS_LEFTY,
	cAXIS_RIGHTX,
	cAXIS_RIGHTY,
	cAXIS_TRIGGERLEFT,
	cAXIS_TRIGGERRIGHT,

	cAXIS_MAX
};

ControllerInput MapSDLGameControllerButton(SDL_GameControllerButton button);
ControllerInput MapSDLGameControllerAxis(SDL_GameControllerAxis axis);

// Convert from an enum to a code that represents an index into a combined range containing all Keyboard/Mouse/Controller inputs
int GetInputCode(KeyboardInput scancode);
int GetInputCode(MouseInput input);
int GetInputCode(ControllerInput input);

// Return a string with the name of the input code, if name starts with '.' it should not be used. On input code, returns "."
const char* GetInputCodeName(int code);

struct event_Key
{
	KeyboardInput keycode;
	char key;

	bool state;
	int repeat;

	bool key_shift;
	bool key_ctrl;
	bool key_alt;
};

struct event_Mouse
{
	int pixel_x, pixel_y;
	float screen_x, screen_y;
	float vel_x, vel_y;

	MouseInput mousecode;

	bool button_left;
	bool button_middle;
	bool button_right;
	bool button_x1;
	bool button_x2;

	int button_repeat;

	// If this is true, this mouse event is from the mouse wheel being scrolled
	// vel_x and vel_y hold the direction of scrolling
	bool is_wheel;
};

struct event_Controller
{
	int controllerId;
	ControllerInput input;
	float value;
};

/**
* An event for inputs
*/
struct event_Input
{
	InputName name;
	union
	{
		float state; // for 1d inputs (buttons)
		vec2 axis;   // for 2d inputs (axes)
	};

	bool enabled() { return state == 1.f; }
};

/**
* An adapter for each input enum. Converts from the enum values to a linear integer range.
*/
struct InputCode
{
	int code;

	operator int() const {
		return code;
	}

	InputCode()                      : code(-1)                  {}
	InputCode(int code)              : code(code)                {}
	InputCode(KeyboardInput input)   : code(GetInputCode(input)) {}
	InputCode(MouseInput input)      : code(GetInputCode(input)) {}
	InputCode(ControllerInput input) : code(GetInputCode(input)) {}
};

namespace std {
	template <> struct hash<InputCode> {
		size_t operator()(const InputCode& code) const {
			return (int)code;
		}
	};
}

/**
*  Settings for each axis.
*  InputAxis and GroupAxis store this so these settings can be applied at both levels.
*/
struct InputAxisSettings
{
    /**
    * Limit the length of output to 1.
    */
	bool limitToUnit = false;

    /**
    * Normalize the length of the output to 1.
    */
	bool normalized = false;

    /**
    * States or axes which have not been used this frame should be read as 0.
    */
    bool useOnlyInputSetThisFrame = false;

    /**
    * When reading an axis, only consider the state with the latest frame.
    */
    bool useOnlyLatestInput = false;

    /**
    * Round to 0 if below this length.
    */
    float deadzone = 0.f;

    /**
    * Masks allow for storing multiple 'control schemes' at once, but only listen to one.
    * If mask is an empty string, it will be accepted by any active mask.
    */
	std::string mask;

    /**
    * A list of user defined functions to transform after the default settings.
    */
    std::vector<std::function<vec2(vec2)>> userPipeline;

	bool operator==(const InputAxisSettings& other) const;
	bool operator!=(const InputAxisSettings& other) const;
};

/**
* An axis is defined as a collection of inputs and weights.
* Using this scheme, it is easy to define X/Y axes, and combo buttons.
* To create multi-bindings like Controller Left Stick / WASD / Arrow keys, use an AxisGroup.
*/
struct InputAxis
{
	std::unordered_map<int, vec2> components;
	InputAxisSettings settings;
                
	bool operator==(const InputAxis& other) const;
	bool operator!=(const InputAxis& other) const;
};

/**
* A summation of input axes. Useful to bind many inputs to the same axis.
* The main use for Group Axes is binding keyboard and analogue controller inputs as
* they need to have separate dead zones before summation. After summation
* the final value is processed again using the group axis' settings.
*/
struct AxisGroup
{
	std::vector<InputName> axes;
	InputAxisSettings settings;
        
	bool operator==(const AxisGroup& other) const;
	bool operator!=(const AxisGroup& other) const;
};

/**
* Allows for Input and Group axis to set their settings without code copy
*/
template<typename _a>
class InputAxisSettingsBuilder
{
public:
    InputAxisSettingsBuilder(InputAxisSettings* settings)
        : m_settings (settings)
    {}
        
    // these should really only be set once, so can only
    // be used to set the flags to the opposite of their
    // defaults.

    _a& LimitToUnit(/*bool limitToUnit*/) {
        m_settings->limitToUnit = true;
        return *(_a*)this;
    }
        
    _a& Normalized(/*bool normalized*/) {
        m_settings->normalized = true;
        return *(_a*)this;
    }
        
    _a& UseOnlyInputSetThisFrame(/*bool useOnlyInputSetThisFrame*/) {
        m_settings->useOnlyInputSetThisFrame = true;
        return *(_a*)this;
    }

    _a& UseOnlyLatestInput(/*bool useOnlyInputSetThisFrame*/) {
        m_settings->useOnlyLatestInput = true;
        return *(_a*)this;
    }

    _a& Deadzone(float deadzone) {
        m_settings->deadzone = deadzone;
        return *(_a*)this;
    }

    _a& Mask(const std::string& mask) {
        m_settings->mask = mask;
        return *(_a*)this;
    }
    
    _a& Process(std::function<vec2(vec2)> function) {
        m_settings->userPipeline.push_back(function);
        return *(_a*)this;
    }

private:
    InputAxisSettings* m_settings;
};

// need to forwards declare so builders can update Mapping

class InputMap;

/**
* Provides a fluent interface for configuring input axes
*/
class InputAxisBuilder : public InputAxisSettingsBuilder<InputAxisBuilder>
{
public:
    InputAxisBuilder(InputMap* map, InputAxis* axis, const InputName& name);
        
    InputAxisBuilder& Map(InputCode code, vec2 weight);
    InputAxisBuilder& MapButton(InputCode code, float weight = 1.f);
        
private:
    InputMap* m_map;
    InputAxis* m_axis;
    InputName m_name;
};

/**
* Provides a fluent interface for configuring axis groups
*/
class AxisGroupBuilder : public InputAxisSettingsBuilder<AxisGroupBuilder>
{
public:
    AxisGroupBuilder(InputMap* map, AxisGroup* axis, const InputName& name);
        
    AxisGroupBuilder& Map(const InputName& name);
        
private:
    InputMap* m_map;
    AxisGroup* m_axis;
    InputName m_name;
};

class InputMap
{
public:
    /**
    * Makes and returns a new input axis.
    * The returned axis is a reference with builder functions to bind inputs.
    */
    InputAxisBuilder CreateAxis(const InputName& name);

    /**
    * Makes and returns a new group axis.
    * The returned axis is a reference with builder functions to bind input axes by name.
    */
    AxisGroupBuilder CreateGroupAxis(const InputName& name);

    /**
    * Return the state of an axis. If an input axis and axis group share names, the group will take precedence and be returned.
    */
    vec2 GetAxis(const InputName& axis);

    /**
    * Buttons are treated as axes with only an x component.
    * Shorthand for GetAxis(button).x
    */
    float GetButton(const InputName& button);

    /**
    * For single press buttons.
    * Once true is returned, false will be returned until the button is released and pressed again.
    */
    bool Once(const InputName& button);

    /**
    * Return true if the last input set was a controller
    */
    bool IsUsingController() const;
    
    /**
    * Set the global active mask. See InputAxisSettings::mask
    */
    void SetActiveMask(const std::string& mask);

    /**
    * If the view port doesn't take up the whole window, use this function to
    * map the screen to view port. This is not fullscreen at the OS level, it's
    * inside the window for split screen / editor windows.
    */
    void SetViewportBounds(vec2 screenMin, vec2 screenSize);

    /**
    * Map a screen coordinate to a view port coordinate. Only needed when view port
    * is a different size than the window
    */
    vec2 MapToViewport(float screenX, float screenY);

    /**
    * Update the states for falloff
    */
    void UpdateStates(float deltaTime);
    
    /**
    * Internal
    */

    void SetActiveFrame(int frame);
    void SetState(int code, float state);
    const std::unordered_set<InputName>& GetMapping(InputCode code);
    float GetRawState(InputCode code);

    InputMap();

private:
    bool _FailsMask(const std::string& mask);
    bool _AxisExists(const InputName& axis);
    bool _GroupAxisExists(const InputName& axis);
    vec2 _GetAxisNoRecurse(const InputName& axisName, bool useOnlyInputSetThisFrameOverride);
    vec2 _ApplySettings(vec2 in, const InputAxisSettings& settings);

private:
    // mapping of name to Group axis
	// these are groups of other axes so each can have its own processing
	// then be combined
	std::unordered_map<InputName, AxisGroup> GroupAxes;

	// mapping of name to axis
	// all buttons can be represented as axes with a dead zone
	// multi-button combos can be thought as an axis with components ('A', .5) and ('B', .5) and a dead zone = 1
	std::unordered_map<InputName, InputAxis> Axes;

	// mapping of code to input name
	// allows us to skip a search through all components of each axis to find a mapping
	std::unordered_map<InputCode, std::unordered_set<InputName>> Mapping;

	// raw states. todo: this should be an array

    struct InputState {
        float value;
        int frameLastUpdated;
    };

	std::unordered_map<int, InputState> State;

	// once states
	std::unordered_set<InputName> OnceButtonIsDown;

	// for mapping mouse into view port
	vec2 ViewportMin;
	vec2 ViewportSize;

	std::string activeMask;
    int activeFrame;
    
    bool lastInputWasController;
    
    // friend so these can access Mapping, seems fishy
    friend class InputAxisBuilder;
    friend class AxisGroupBuilder;
};
