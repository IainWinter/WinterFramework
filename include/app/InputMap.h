#pragma once

#include "Common.h"
#include "util/context.h"
#include <unordered_map>
#include <unordered_set>
#include <array>

//
// some oddities
//		tofix: Mappings can be overwritten if two Group axes use the same buttons

/**
 * A type to identify axes
 */
using InputName = std::string;

/**
 * A type to identify masks
 */
using InputMask = std::string;

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

namespace Input
{
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
}

namespace std {
	template <> struct hash<Input::InputCode> {
		size_t operator()(const Input::InputCode& code) const {
			return (int)code;
		}
	};
}

namespace Input
{
    /**
     *  Settings for each axis.
     *  InputAxis and GroupAxis store this so these settings can be applied at both levels.
     */
	struct InputAxisSettings
	{
        /**
         * Round to 0 if below this length.
         */
		float deadzone = 0.f;

        /**
         * Limit the length of output to 1.
         */
		bool limitToUnit = true;

        /**
         * Normalize the length of the output to 1.
         */
		bool normalized = false;

        /**
         * Masks allow for storing multiple 'control schemes' at once, but only listen to one.
         * If mask is an empty string, it will be accepted by any active mask.
         */
		std::string mask;

		bool operator==(const InputAxisSettings& other) const;
		bool operator!=(const InputAxisSettings& other) const;
	};

    /**
     * An axis is defined as a collection of inputs and weights.
     * Using this scheme it is easy to define X/Y axes, and combo buttons.
     * To create multi-bindings like Controller Left Stick / WASD / Arrow keys, see AxisGroup.
     */
	struct InputAxis
	{
		std::unordered_map<int, vec2> components; // (code, component) -> sum of State[code] * component is axis state
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
        
        _a& Deadzone(float deadzone) {
            m_settings->deadzone = deadzone;
            return *(_a*)this;
        }
        
        _a& LimitToUnit(bool limitToUnit) {
            m_settings->limitToUnit = limitToUnit;
            return *(_a*)this;
        }
        
        _a& Normalized(bool normalized) {
            m_settings->normalized = normalized;
            return *(_a*)this;
        }
        
        _a& Mask(const std::string& mask) {
            m_settings->mask = mask;
            return *(_a*)this;
        }
        
    private:
        InputAxisSettings* m_settings;
    };

    /**
     * Provides a fluent interface for configuring input axes
     */
    class InputAxisBuilder : public InputAxisSettingsBuilder<InputAxisBuilder>
    {
    public:
        InputAxisBuilder(InputAxis* axis, const InputName& name);
        
        InputAxisBuilder& Map(InputCode code, vec2 weight);
        InputAxisBuilder& MapButton(InputCode code, float weight = 1.f);
        
    private:
        InputAxis* m_axis;
        InputName m_name;
    };

    /**
     * Provides a fluent interface for configuring axis groups
     */
    class AxisGroupBuilder : public InputAxisSettingsBuilder<AxisGroupBuilder>
    {
    public:
        AxisGroupBuilder(AxisGroup* axis, const InputName& name);
        
        AxisGroupBuilder& Map(const InputName& name);
        
    private:
        AxisGroup* m_axis;
        InputName m_name;
    };

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
     * Serialize an input context to a file
     */
    void WriteInputPack(const std::string& filename);

    /**
     * Deserialize an input context from a file
     */
    void ReadInputPack(const std::string& filename);


    /**
     *     Internal
     */


    void SetState(int code, float state);
    const InputName& GetMapping(InputCode code);

	struct InputContext : wContext
	{
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
		std::unordered_map<InputCode, InputName> Mapping;

		// raw states
		std::unordered_map<int, float> State;

		// once states
		std::unordered_set<InputName> OnceButtonIsDown;

		// for mapping mouse into view port
		vec2 ViewportMin;
		vec2 ViewportSize;

		// active mask
		std::string activeMask;

		InputContext();
	};

	wContextDecl(InputContext);
}
